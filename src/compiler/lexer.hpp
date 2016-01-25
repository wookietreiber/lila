/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#ifndef LILA_LEXER_H
#define LILA_LEXER_H

#include <llvm/ADT/STLExtras.h>

#include "token.hpp"

using namespace lila::token;

namespace lila {
  namespace lexer {

    void tokenize(basic_istream<char>* is, vector<unique_ptr<Token>>* tokens);

  }
}

#endif
