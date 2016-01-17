/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include <fstream>
#include <getopt.h>
#include <iostream>

#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Bitcode/ReaderWriter.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "codegen.hpp"
#include "parser.hpp"

using namespace lila::codegen;
using namespace lila::parser;

int main(int argc, char** argv) {

  const char *input = "-";
  const char *output = "a.out.bc";

  bool verbose = false;

  // ---------------------------------------------------------------------------
  // parse command line options
  // ---------------------------------------------------------------------------

  char usage [1024];
  snprintf(usage,
           1024,
           "%s\n"
           "\n"
           "usage: lilac [OPTIONS] INPUT\n"
           "\n"
           "options:\n"
           "    -o FILENAME      write output to FILENAME\n"
           "                     if omitted, writes to %s\n"
           "    -v               verbose output\n"
           "    INPUT            read source code from INPUT\n"
           "                     if omitted or %s, reads from STDIN\n"
           "\n",
           PACKAGE_STRING,
           output,
           input
           );

  int c;
  while ((c = getopt (argc, argv, "ho:v")) != -1)
    switch (c) {
    case 'h':
      cout << usage;
      return 0;
    case 'o':
      output = optarg;
      break;
    case 'v':
      verbose = true;
      break;
    default:
      cerr << usage;
      return 1;
    }

  if (optind < argc) {
    input = argv[optind];
  }

  // ---------------------------------------------------------------------------
  // read code and tokenize it
  // ---------------------------------------------------------------------------

  vector<unique_ptr<Token>> tokens;

  if (strcmp(input, "-") == 0) {
    tokenize(&cin, &tokens);
  } else {
    ifstream is(input);

    if (is) {
      tokenize(&is, &tokens);
      is.close();
    } else {
      cerr << "error opening file: " << input << endl;
      return 1;
    }
  }

  if (verbose)
    for (auto it = tokens.begin() ; it != tokens.end(); ++it) {
      Token * token = it->get();
      cerr << "[debug] [token] \"" << token->toString() << "\"" << endl;
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

  if (verbose)
    cerr << "[debug] [ast] " << ast->toString() << endl;

  // ---------------------------------------------------------------------------
  // generate LLVM IR code
  // ---------------------------------------------------------------------------

  CodeGen codegen("lilamodule", llvm::getGlobalContext());

  codegen.generateCode(move(ast));

  // ---------------------------------------------------------------------------
  // write LLVM IR code
  // ---------------------------------------------------------------------------

  std::error_code ec;
  llvm::raw_fd_ostream out(output, ec, llvm::sys::fs::F_None);

  if (ec) {
    cerr << "error: " << output << ": " << ec.message() << endl;
    return ec.value();
  }

  llvm::WriteBitcodeToFile(codegen.module.get(), out);

  // ---------------------------------------------------------------------------
  // end
  // ---------------------------------------------------------------------------

  return 0;
}
