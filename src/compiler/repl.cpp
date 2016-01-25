/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include "repl.hpp"

#include <sstream>

#include <llvm/Support/raw_os_ostream.h>

void lila::repl(istream &replin, ostream &replout, ostream &replerr) {
  string line;
  bool verbose = false;
  auto jit = llvm::make_unique<lila::jit::LilaJIT>();

  replout << "welcome to " << PACKAGE_STRING << endl;
  replout << "type in expressions to have them evaluated" << endl;
  replout << "type :help for more information" << endl << endl;

  while (1) {
    replout << "lila> ";

    getline(replin, line);

    if (replin.eof()) {
      // write final newline so shell prompt does not end up after our prompt
      replout << ":quit" << endl;
      break;
    }

    if (line.compare(":quit") == 0) {
      break;
    }

    if (line.compare(":verbose") == 0) {
      verbose = true;
      continue;
    }

    if (line.compare(":silent") == 0) {
      verbose = false;
      continue;
    }

    if (line.compare(":help") == 0) {
      replout << ":help        print this help summary"    << endl;
      replout << ":silent      do not print tokens/ast/ir" << endl;
      replout << ":verbose     print tokens/ast/ir"        << endl;
      replout << ":quit        exit"                       << endl;
      continue;
    }

    // tokenize the line
    istringstream streamed(line);
    vector<unique_ptr<Token>> tokens;
    tokenize(&streamed, &tokens);

    if (verbose)
      for (auto it = tokens.begin() ; it != tokens.end(); ++it) {
        Token * token = it->get();
        replerr << "[token] \"" << token->toString() << "\"" << endl;
      }

    // try parse tokens to ast
    Parser parser(&tokens);
    unique_ptr<ASTNode> ast;
    try {
      ast = parser.parse();

      if (verbose)
        replerr << "[ast] " << ast->toString() << endl;
    } catch (const char * msg) {
      replerr << msg << endl;
      // TODO if expression incomplete, i.e. could continue,
      // TODO do NOT clear line/tokens, read next line and try again
      continue;
    }

    // TODO pattern match on ast (may not always be top level expression)

    CodeGen codegen("lilamodule", llvm::getGlobalContext());
    codegen.generateCode(move(ast), false);

    if (verbose) {
      llvm::raw_os_ostream llvmreplerr(replerr);
      codegen.module->print(llvmreplerr, nullptr);
    }

    codegen.module->setDataLayout(jit->getTargetMachine().createDataLayout());
    auto moduleHandle = jit->addModule(move(codegen.module));

    auto symbol = jit->findSymbol("anonymous");
    assert(symbol && "function not found");

    double (*FP)() = (double (*)())(intptr_t)symbol.getAddress();
    replout << FP() << endl;

    jit->removeModule(moduleHandle);
    // TODO keep result in jit for later reference
  }
}
