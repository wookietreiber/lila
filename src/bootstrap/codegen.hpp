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
      vector<string> curscope;
      map<string, map<string, llvm::Value*> > scoped_values;

      void removeScope() {
        curscope.pop_back();
      }

      string scopestr() {
        return util::mkString(curscope, ".");
      }

      void addScope(string level) {
        // first add to curscope
        curscope.push_back(level);

        // second add empty values to scope map
        map<string, llvm::Value*> values;
        scoped_values[scopestr()] = values;
      }

      void addScopedValue(string name, llvm::Value* value) {
        string scope = scopestr();

        auto values = scoped_values[scope];
        values[name] = value;

        scoped_values[scope] = values;
      }

      llvm::Value * findScopedValue(string name) {
        // search from current scope outwards
        for (int i = curscope.size(); i >= 0; i--) {
          ostringstream oss;
          for (int j = 0; j < i; j++) {
            oss << curscope[j];
            if (j < (i - 1)) oss << '.';
          }

          string scope = oss.str();

          map<string, llvm::Value*> values = scoped_values[scope];

          for (auto it = values.begin() ; it != values.end(); ++it)
            if (name == it->first) return it->second;
        }

        return nullptr;
      }

    public:
      CodeGen(string modulename, llvm::LLVMContext& ctx)
        : context(ctx), Builder(llvm::IRBuilder<>(ctx)) {
        module = llvm::make_unique<llvm::Module>(modulename, context);
      }

      llvm::Value * generateCodeExpr(ExprAST *ast);
      llvm::Value * generateCodeNumber(NumberExprAST *ast);
      llvm::Value * generateCodeBlock(BlockAST *ast);
      llvm::Value * generateCodeBinOp(BinaryExprAST *ast);
      llvm::Value * generateCodeCall(CallAST *ast);
      llvm::Value * generateCodeValue(ValueAST *ast);
      llvm::Function * generateCodeDef(DefAST *ast);

      int wrapTopLevelBlockInMain(BlockAST *ast);

      unique_ptr<CodegenResult> generateCode(unique_ptr<ASTNode> ast);
    };

  }
}

#endif
