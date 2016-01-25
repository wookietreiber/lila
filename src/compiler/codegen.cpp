/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
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
      } else if (auto x = dynamic_cast<BinaryExprAST*>(ast)) {
        return generateCodeBinOp(x);
      } else
        throw "oops";
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
        throw "unknown operator";
      }

      return nullptr;
    }

    llvm::Function * CodeGen::wrapInFunc(llvm::Value *code, string name) {
      // Make the function type:  double(void)
      vector<llvm::Type *> args;
      llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(context), args, false);

      llvm::Function *TheFunction =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, module.get());

      // Create a new basic block to start insertion into.
      llvm::BasicBlock *BB = llvm::BasicBlock::Create(context, "entry", TheFunction);
      Builder.SetInsertPoint(BB);

      Builder.CreateRet(code);

      verifyFunction(*TheFunction);

      return TheFunction;
    }

    void CodeGen::wrapInMain(llvm::Value *code) {
      // generate a function
      llvm::Function * TheFunction = wrapInFunc(code, "anonymous");

      // generate void main()
      llvm::FunctionType *voidType = llvm::FunctionType::get(Builder.getVoidTy(), false);
      llvm::Function *mainFunc =
        llvm::Function::Create(voidType, llvm::Function::ExternalLinkage, "main", module.get());

      llvm::BasicBlock *MainBlock = llvm::BasicBlock::Create(context, "mainentry1", mainFunc);
      Builder.SetInsertPoint(MainBlock);

      // call the function inside main
      std::vector<llvm::Value *> ArgsV;

      auto bippy = Builder.CreateCall(TheFunction, ArgsV, "calltmp");

      llvm::Value *fmt = Builder.CreateGlobalStringPtr("%f\n");

      vector<llvm::Type *> putsArgs;
      putsArgs.push_back(Builder.getInt8Ty()->getPointerTo());
      llvm::ArrayRef<llvm::Type*> argsRef(putsArgs);

      llvm::FunctionType *putsType =
        llvm::FunctionType::get(Builder.getInt32Ty(), argsRef, true);
      llvm::Constant *putsFunc = module->getOrInsertFunction("printf", putsType);

      vector<llvm::Value *> CallArgs;
      CallArgs.push_back(fmt);
      CallArgs.push_back(bippy);
      Builder.CreateCall(putsFunc, CallArgs, "oo");

      Builder.CreateRetVoid();

      // Validate the generated code, checking for consistency.
      verifyFunction(*mainFunc);
    }

    void CodeGen::generateCode(unique_ptr<ASTNode> ast, bool wrap) {
      if (auto x = dynamic_cast<NumberExprAST*>(ast.get())) {
        auto code = generateCodeNumber(x);
        if (wrap) wrapInMain(code); else wrapInFunc(code, "anonymous");
      } else if (auto x = dynamic_cast<BinaryExprAST*>(ast.get())) {
        auto code = generateCodeBinOp(x);
        if (wrap) wrapInMain(code); else wrapInFunc(code, "anonymous");
      } else
        throw "can't handle ast";
    }

  }
}
