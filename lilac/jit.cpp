/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include "jit.hpp"

namespace lila {
  namespace jit {

    template <typename T> static vector<T> singletonSet(T t) {
      vector<T> v;
      v.push_back(move(t));
      return v;
    }

    string LilaJIT::mangle(const string &name) {
      string mangled;
      llvm::raw_string_ostream os(mangled);
      llvm::Mangler::getNameWithPrefix(os, name, dataLayout);
      return mangled;
    }

    llvm::orc::JITSymbol LilaJIT::findMangledSymbol(const string &name) {
      for (auto handle : llvm::make_range(moduleHandles.rbegin(), moduleHandles.rend()))
        if (auto symbol = compileLayer.findSymbolIn(handle, name, true))
          return symbol;

      if (auto symbol = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name))
        return llvm::orc::JITSymbol(symbol, llvm::JITSymbolFlags::Exported);

      return nullptr;
    }

    ModuleHandleT LilaJIT::addModule(unique_ptr<llvm::Module> module) {
      auto resolver = llvm::orc::createLambdaResolver(
        [&](const string &name) {
          if (auto symbol = findMangledSymbol(name))
            return llvm::RuntimeDyld::SymbolInfo(symbol.getAddress(), symbol.getFlags());

          return llvm::RuntimeDyld::SymbolInfo(nullptr);
        },

        [](const string &s) {
          return nullptr;
        }
      );

      auto handle = compileLayer.addModuleSet(
        singletonSet(move(module)),
        llvm::make_unique<llvm::SectionMemoryManager>(),
        move(resolver)
      );

      moduleHandles.push_back(handle);

      return handle;
    }

    void LilaJIT::removeModule(ModuleHandleT handle) {
      moduleHandles.erase(find(moduleHandles.begin(), moduleHandles.end(), handle));
      compileLayer.removeModuleSet(handle);
    }

  }
}
