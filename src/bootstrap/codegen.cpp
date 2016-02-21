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
      // TODO handle block with scopes
      } else {
        error = "can't handle expression ast";
        return nullptr;
      }
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
      vector<llvm::Type*> args(ast->args.size(), llvm::Type::getDoubleTy(context));
      llvm::FunctionType *funcType =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(context), args, false);

      llvm::Function * func =
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, ast->name, module.get());

      unsigned i = 0;
      for (auto funcargs = func->arg_begin(); i != ast->args.size(); ++funcargs, ++i) {
        funcargs->setName(ast->args[i]);
        values[ast->args[i]] = funcargs;
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

        return func;
      } else {
        func->eraseFromParent();
        error = "error creating function body: " + error;
        return nullptr;
      }
    }

    llvm::Value * CodeGen::generateCodeValue(ValueAST *ast) {
      llvm::Value * exprCode = generateCodeExpr(ast->expr.get());
      values[ast->name] = exprCode;
      return exprCode;
    }

    llvm::Value * CodeGen::generateCodeCall(CallAST *ast) {
      if (auto value = values[ast->name]) {
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

    int CodeGen::wrapTopLevelBlockInMain(BlockAST *ast) {
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
          if (!code) return 0;

        } else if (auto x = dynamic_cast<DefAST*>(node)) {
          generateCodeDef(x);
          Builder.SetInsertPoint(MainBlock);

        } else if (auto x = dynamic_cast<NumberExprAST*>(node)) {
          auto code = generateCodeNumber(x);
          if (!code) return 0;
          lastExpr = code;

        } else if (auto x = dynamic_cast<BinaryExprAST*>(node)) {
          auto code = generateCodeBinOp(x);
          if (!code) return 0;
          lastExpr = code;

        } else if (auto x = dynamic_cast<CallAST*>(node)) {
          auto code = generateCodeCall(x);
          if (!code) return 0;
          lastExpr = code;

        } else {
          error = "can't handle ast";
          return 0;
        }
      }

      llvm::Value *fmt = Builder.CreateGlobalStringPtr("%f\n");

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
        return 0;
      }

      return 1;
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
