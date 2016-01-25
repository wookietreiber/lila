/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#ifndef LILA_REPL_H
#define LILA_REPL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "codegen.hpp"
#include "jit.hpp"
#include "parser.hpp"

#include <llvm/Support/TargetSelect.h>

using namespace lila::codegen;
using namespace lila::parser;

namespace lila {

  void repl(istream &replin, ostream &replout, ostream &replerr);

}

#endif
