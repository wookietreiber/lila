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

    class LexerResult {
    public:
      virtual ~LexerResult() {}
    };

    class LexerSuccess : public LexerResult {
    public:
      unique_ptr<vector<unique_ptr<Token>>> tokens;
      explicit LexerSuccess(unique_ptr<vector<unique_ptr<Token>>> tokens) : tokens(move(tokens)) {}
    };

    class LexerFailure : public LexerResult {
    public:
      string msg;
      explicit LexerFailure(string msg) : msg(msg) {}
    };

    unique_ptr<LexerResult> tokenize(basic_istream<char>* is);

  }
}

#endif
