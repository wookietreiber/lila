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

    auto lexerResult = tokenize(&streamed);

    if (auto failure = dynamic_cast<LexerFailure*>(lexerResult.get())) {
      replerr << "[lexer] [error] " << failure->msg << endl;
      continue;
    }

    LexerSuccess * lexsuccess = dynamic_cast<LexerSuccess*>(lexerResult.get());
    vector<unique_ptr<Token>> * tokens = lexsuccess->tokens.get();

    if (verbose)
      for (auto it = tokens->begin() ; it != tokens->end(); ++it) {
        Token * token = it->get();
        replerr << "[token] \"" << token->toString() << "\"" << endl;
      }

    // try parse tokens to ast
    Parser parser(tokens);

    auto parserResult = parser.parse();

    if (auto failure = dynamic_cast<ParserFailure*>(parserResult.get())) {
      // TODO if expression incomplete, i.e. could continue,
      // TODO do NOT clear line/tokens, read next line and try again
      replerr << "[parser] [error] " << failure->msg << endl;
      continue;
    }

    auto parsesuccess = dynamic_cast<ParserSuccess*>(parserResult.get());
    auto ast = move(parsesuccess->ast);

    if (verbose)
      replerr << "[ast] " << ast->toString() << endl;

    // TODO pattern match on ast (may not always be top level expression)

    CodeGen codegen("lilamodule", llvm::getGlobalContext());

    auto cgresult = codegen.generateCode(move(ast), true);

    if (auto failure = dynamic_cast<CodegenFailure*>(cgresult.get())) {
      replerr << "[codegen] [error] " << failure->msg << endl;
      continue;
    }

    auto cgsuccess = dynamic_cast<CodegenSuccess*>(cgresult.get());
    auto module = move(cgsuccess->module);

    if (verbose) {
      llvm::raw_os_ostream llvmreplerr(replerr);
      module->print(llvmreplerr, nullptr);
    }

    module->setDataLayout(jit->getTargetMachine().createDataLayout());
    auto moduleHandle = jit->addModule(move(module));

    auto symbol = jit->findSymbol("anonymous");
    assert(symbol && "function not found");

    double (*FP)() = (double (*)())(intptr_t)symbol.getAddress();
    replout << FP() << endl;

    jit->removeModule(moduleHandle);
    // TODO keep result in jit for later reference
  }
}
