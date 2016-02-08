/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include "codegen.hpp"

namespace lila {
  namespace codegen {

    llvm::Value* CodeGen::generateCodeNumber(NumberExprAST *ast) {
      return llvm::ConstantFP::get(context, llvm::APFloat(ast->Val));
    }

    llvm::Value* CodeGen::generateCodeExpr(ExprAST *ast) {
      if (auto x = dynamic_cast<NumberExprAST*>(ast)) {
        return generateCodeNumber(x);
      } else if (auto x = dynamic_cast<CallValueAST*>(ast)) {
        return generateCodeCallValue(x);
      } else if (auto x = dynamic_cast<BinaryExprAST*>(ast)) {
        return generateCodeBinOp(x);
      } else {
        error = "can't handle expression ast";
        return nullptr;
      }
    }

    llvm::Value* CodeGen::generateCodeBinOp(BinaryExprAST *ast) {
      llvm::Value *L = generateCodeExpr(ast->LHS.get());
      llvm::Value *R = generateCodeExpr(ast->RHS.get());

      if (!L || !R)
        return nullptr;

      string op = ast->Op;

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

    llvm::Value * CodeGen::generateCodeValue(ValueAST *ast) {
      llvm::Value * exprCode = generateCodeExpr(ast->expr.get());
      values[ast->name] = exprCode;
      return exprCode;
    }

    llvm::Value * CodeGen::generateCodeCallValue(CallValueAST *ast) {
      return values[ast->name];
    }

    int CodeGen::wrapTopLevelBlockInMain(BlockAST *ast) {
      vector<unique_ptr<ASTNode>> * body = ast->body.get();

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
        llvm::Value * code;

        if (auto x = dynamic_cast<ValueAST*>(node)) {
          code = generateCodeValue(x);

        } else if (auto x = dynamic_cast<NumberExprAST*>(node)) {
          code = generateCodeNumber(x);
          lastExpr = code;

        } else if (auto x = dynamic_cast<BinaryExprAST*>(node)) {
          code = generateCodeBinOp(x);
          lastExpr = code;

        } else if (auto x = dynamic_cast<CallValueAST*>(node)) {
          code = generateCodeCallValue(x);
          lastExpr = code;

        } else {
          error = "can't handle ast";
          return 0;
        }

        if (!code) return 0;
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

      // Validate the generated code, checking for consistency.
      verifyFunction(*mainFunc);

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