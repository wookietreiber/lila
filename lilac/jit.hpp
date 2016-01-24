/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#ifndef LILA_JIT_H
#define LILA_JIT_H

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Support/DynamicLibrary.h>

using namespace std;

namespace lila {
  namespace jit {

    typedef llvm::orc::ObjectLinkingLayer<> ObjLayerT;
    typedef llvm::orc::IRCompileLayer<ObjLayerT> CompileLayerT;
    typedef CompileLayerT::ModuleSetHandleT ModuleHandleT;

    class LilaJIT {
    private:
      unique_ptr<llvm::TargetMachine> targetMachine;
      const llvm::DataLayout dataLayout;
      ObjLayerT objectLayer;
      CompileLayerT compileLayer;
      vector<ModuleHandleT> moduleHandles;

      string mangle(const string &name);
      llvm::orc::JITSymbol findMangledSymbol(const string &name);

    public:
      LilaJIT()
        : targetMachine(llvm::EngineBuilder().selectTarget()),
          dataLayout(targetMachine->createDataLayout()),
          compileLayer(objectLayer, llvm::orc::SimpleCompiler(*targetMachine)) {
        llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
      }

      ModuleHandleT addModule(unique_ptr<llvm::Module> module);
      void removeModule(ModuleHandleT handle);

      llvm::TargetMachine &getTargetMachine() {
        return *targetMachine;
      }

      llvm::orc::JITSymbol findSymbol(const string name) {
        return findMangledSymbol(mangle(name));
      }
    };

  }
}

#endif
