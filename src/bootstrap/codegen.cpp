/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include "codegen.hpp"

#include <llvm/Support/raw_ostream.h>

namespace lila {
  namespace codegen {

    llvm::Value* CodeGen::generateCodeNumber(NumberExprAST *ast) {
      return llvm::ConstantFP::get(context, llvm::APFloat(ast->value));
    }

    llvm::Value* CodeGen::generateCodeExpr(ExprAST *ast) {
      if (auto x = dynamic_cast<NumberExprAST*>(ast)) {
        return generateCodeNumber(x);
      } else if (auto x = dynamic_cast<CallAST*>(ast)) {
        return generateCodeCall(x);
      } else if (auto x = dynamic_cast<BinaryExprAST*>(ast)) {
        return generateCodeBinOp(x);
      } else if (auto x = dynamic_cast<BlockAST*>(ast)) {
        return generateCodeBlock(x);
      } else {
        error = "can't handle expression ast";
        return nullptr;
      }
    }

    llvm::Value* CodeGen::generateCodeBlock(BlockAST *ast) {
      llvm::BasicBlock * prevInsertPoint = Builder.GetInsertBlock();
      llvm::Value * lastExpr;

      for (auto it = ast->body->begin() ; it != ast->body->end(); ++it) {
        ASTNode * node = it->get();

        if (auto x = dynamic_cast<ValueAST*>(node)) {
          auto code = generateCodeValue(x);
          if (!code) return nullptr;

        } else if (auto x = dynamic_cast<DefAST*>(node)) {
          generateCodeDef(x);
          Builder.SetInsertPoint(prevInsertPoint);

        } else if (auto x = dynamic_cast<NumberExprAST*>(node)) {
          auto code = generateCodeNumber(x);
          if (!code) return nullptr;
          lastExpr = code;

        } else if (auto x = dynamic_cast<BinaryExprAST*>(node)) {
          auto code = generateCodeBinOp(x);
          if (!code) return nullptr;
          lastExpr = code;

        } else if (auto x = dynamic_cast<CallAST*>(node)) {
          auto code = generateCodeCall(x);
          if (!code) return nullptr;
          lastExpr = code;

        } else {
          error = "can't handle ast";
          return nullptr;
        }
      }

      return lastExpr;
    }

    llvm::Value* CodeGen::generateCodeBinOp(BinaryExprAST *ast) {
      llvm::Value *L = generateCodeExpr(ast->lhs.get());
      llvm::Value *R = generateCodeExpr(ast->rhs.get());

      if (!L || !R)
        return nullptr;

      string op = ast->op;

      if (op == "+") {
        return Builder.CreateFAdd(L, R, "addtmp");
      } else if (op == "-") {
        return Builder.CreateFSub(L, R, "subtmp");
      } else if (op == "*") {
        return Builder.CreateFMul(L, R, "multmp");
      } else {
        error = "unknown operator, don't know what to do with it";
      }

      return nullptr;
    }

    llvm::Function * CodeGen::generateCodeDef(DefAST *ast) {
      // add def name to scope
      addScope(ast->name);

      // argument types
      vector<llvm::Type*> args(ast->args.size(), llvm::Type::getDoubleTy(context));

      // return type
      auto retType = llvm::Type::getDoubleTy(context);

      // complete function type
      llvm::FunctionType *funcType = llvm::FunctionType::get(retType, args, false);

      llvm::Function * func =
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, ast->name, module.get());

      unsigned i = 0;
      for (auto funcarg = func->arg_begin(); i != ast->args.size(); ++funcarg, ++i) {
        funcarg->setName(ast->args[i]);
        addScopedValue(ast->args[i], funcarg);
      }

      llvm::BasicBlock *block = llvm::BasicBlock::Create(context, "entry", func);
      Builder.SetInsertPoint(block);

      if (llvm::Value * result = generateCodeExpr(ast->body.get())) {
        Builder.CreateRet(result);

        string verifyS;
        llvm::raw_string_ostream verifyE(verifyS);
        if (verifyFunction(*func, &verifyE)) {
          func->eraseFromParent();
          error = "something wrong with function \"" + ast->name + "\": " + verifyS;
          return nullptr;
        }

        // remove the def name scope
        removeScope();
        return func;
      } else {
        func->eraseFromParent();
        error = "error creating function body: " + error;
        return nullptr;
      }
    }

    llvm::Value * CodeGen::generateCodeValue(ValueAST *ast) {
      addScope(ast->name);
      llvm::Value * exprCode = generateCodeExpr(ast->expr.get());
      removeScope();

      addScopedValue(ast->name, exprCode);

      return exprCode;
    }

    llvm::Value * CodeGen::generateCodeCall(CallAST *ast) {
      if (auto value = findScopedValue(ast->name)) {
        return value;
      } else if (auto function = module->getFunction(ast->name)) {
        if (function->arg_size() != ast->args->size()) {
          error = "incorrect number of arguments";
          return nullptr;
        }

        vector<llvm::Value*> args;
        for (unsigned i = 0, e = ast->args->size(); i != e; ++i) {
          ExprAST * expr = ast->args->at(i).get();
          llvm::Value * value = generateCodeExpr(expr);
          if (!value) return nullptr;
          args.push_back(value);
        }

        return Builder.CreateCall(function, args, "call" + ast->name);
      } else {
        error = ast->name + " not found";
        return nullptr;
      }
    }

    bool CodeGen::wrapTopLevelBlockInMain(BlockAST *ast) {
      // add main to scope
      addScope("main");

      vector<unique_ptr<ASTNode> > * body = ast->body.get();

      // generate void main()
      llvm::FunctionType *voidType = llvm::FunctionType::get(Builder.getVoidTy(), false);
      llvm::Function *mainFunc =
        llvm::Function::Create(voidType, llvm::Function::ExternalLinkage, "main", module.get());

      // basic block for the top level block
      llvm::BasicBlock *MainBlock = llvm::BasicBlock::Create(context, "entry", mainFunc);
      Builder.SetInsertPoint(MainBlock);

      llvm::Value * lastExpr;

      for (auto it = body->begin() ; it != body->end(); ++it) {
        ASTNode * node = it->get();

        if (auto x = dynamic_cast<ValueAST*>(node)) {
          auto code = generateCodeValue(x);
          if (!code) return false;

        } else if (auto x = dynamic_cast<DefAST*>(node)) {
          generateCodeDef(x);
          Builder.SetInsertPoint(MainBlock);

        } else if (auto x = dynamic_cast<NumberExprAST*>(node)) {
          auto code = generateCodeNumber(x);
          if (!code) return false;
          lastExpr = code;

        } else if (auto x = dynamic_cast<BinaryExprAST*>(node)) {
          auto code = generateCodeBinOp(x);
          if (!code) return false;
          lastExpr = code;

        } else if (auto x = dynamic_cast<CallAST*>(node)) {
          auto code = generateCodeCall(x);
          if (!code) return false;
          lastExpr = code;

        } else {
          error = "can't handle ast";
          return false;
        }
      }

      llvm::Value *fmt = Builder.CreateGlobalStringPtr("%lg\n");

      vector<llvm::Type *> putsArgs;
      putsArgs.push_back(Builder.getInt8Ty()->getPointerTo());
      llvm::ArrayRef<llvm::Type*> argsRef(putsArgs);

      llvm::FunctionType *putsType =
        llvm::FunctionType::get(Builder.getInt32Ty(), argsRef, true);
      llvm::Constant *putsFunc = module->getOrInsertFunction("printf", putsType);

      vector<llvm::Value *> CallArgs;
      CallArgs.push_back(fmt);
      CallArgs.push_back(lastExpr);
      Builder.CreateCall(putsFunc, CallArgs);

      Builder.CreateRetVoid();

      string verifyS;
      llvm::raw_string_ostream verifyE(verifyS);
      if (verifyFunction(*mainFunc, &verifyE)) {
        mainFunc->eraseFromParent();
        error = "something wrong with auto-generated main function: " + verifyS;
        return false;
      }

      // remove the main scope
      removeScope();

      return true;
    }

    unique_ptr<CodegenResult> CodeGen::generateCode(unique_ptr<ASTNode> ast) {
      // top level block
      if (auto block = dynamic_cast<BlockAST*>(ast.get())) {
        if (!wrapTopLevelBlockInMain(block)) {
          auto failure = llvm::make_unique<CodegenFailure>(error);
          return move(failure);
        }

      } else {
        auto failure = llvm::make_unique<CodegenFailure>("can't handle ast");
        return move(failure);
      }

      auto success = llvm::make_unique<CodegenSuccess>(move(module));
      return move(success);
    }

  }
}
