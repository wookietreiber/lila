/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#ifndef LILA_CODEGEN_H
#define LILA_CODEGEN_H

#include <memory>

#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

#include "ast.hpp"

using namespace lila::ast;

namespace lila {
  namespace codegen {

    class CodegenResult {
    public:
      virtual ~CodegenResult() {}
    };

    class CodegenSuccess : public CodegenResult {
    public:
      unique_ptr<llvm::Module> module;
      explicit CodegenSuccess(unique_ptr<llvm::Module> module) : module(move(module)) {}
    };

    class CodegenFailure : public CodegenResult {
    public:
      string msg;
      explicit CodegenFailure(string msg) : msg(msg) {}
    };

    class CodeGen {
      llvm::LLVMContext& context;
      llvm::IRBuilder<> Builder;
      unique_ptr<llvm::Module> module;
      string error;
      map<string, llvm::Value*> values;

    public:
      CodeGen(string modulename, llvm::LLVMContext& ctx)
        : context(ctx), Builder(llvm::IRBuilder<>(ctx)) {
        module = llvm::make_unique<llvm::Module>(modulename, context);
      }

      llvm::Value * generateCodeExpr(ExprAST *ast);
      llvm::Value * generateCodeNumber(NumberExprAST *ast);
      llvm::Value * generateCodeBinOp(BinaryExprAST *ast);
      llvm::Value * generateCodeCallValue(CallValueAST *ast);
      llvm::Value * generateCodeValue(ValueAST *ast);

      int wrapTopLevelBlockInMain(BlockAST *ast);

      unique_ptr<CodegenResult> generateCode(unique_ptr<ASTNode> ast);
    };

  }
}

#endif
