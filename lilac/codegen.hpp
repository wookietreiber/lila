/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
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

    class CodeGen {
      llvm::LLVMContext& context;
      llvm::IRBuilder<> Builder;

    public:
      unique_ptr<llvm::Module> module;

      CodeGen(string modulename, llvm::LLVMContext& ctx)
        : context(ctx), Builder(llvm::IRBuilder<>(ctx)) {
        module = llvm::make_unique<llvm::Module>(modulename, context);
      }

      llvm::Value * generateCodeExpr(ExprAST *ast);
      llvm::Value * generateCodeNumber(NumberExprAST *ast);
      llvm::Value * generateCodeBinOp(BinaryExprAST *ast);

      void wrapInMain(llvm::Value *code);

      void generateCode(unique_ptr<ASTNode> ast);
    };

  }
}

#endif
