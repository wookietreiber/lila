/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include <fstream>
#include <iostream>

#include "codegen.hpp"
#include "parser.hpp"

using namespace lila::codegen;
using namespace lila::parser;

int main(int argc, char** argv) {

  // ---------------------------------------------------------------------------
  // read code and tokenize it
  // ---------------------------------------------------------------------------

  vector<unique_ptr<Token>> tokens;

  if (argc == 1) {
    tokenize(&cin, &tokens);
  } else {
    char * filename = argv[1];

    ifstream is(filename);

    if (is) {
      tokenize(&is, &tokens);
      is.close();
    } else {
      fprintf(stderr, "error opening file\n");
      return 1;
    }
  }

  for (auto it = tokens.begin() ; it != tokens.end(); ++it) {
    Token * token = (*it).get();
    cerr << "[debug] [token] \"" << token->toString().c_str() << "\"" << endl;
  }

  // ---------------------------------------------------------------------------
  // parse the tokens to AST
  // ---------------------------------------------------------------------------

  Parser parser(&tokens);

  unique_ptr<ASTNode> ast;

  try {
    ast = parser.parse();
  } catch (const char * msg) {
    cerr << msg << endl;
    return 1;
  }

  cerr << "[debug] [ast] " << (*ast).toString() << endl;

  // ---------------------------------------------------------------------------
  // generate LLVM IR code
  // ---------------------------------------------------------------------------

  CodeGen codegen("lilamodule", llvm::getGlobalContext());

  codegen.generateCode(move(ast));

  codegen.module->dump();

  // ---------------------------------------------------------------------------
  // end
  // ---------------------------------------------------------------------------

  return 0;
}
