/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include "lexer.hpp"

namespace lila {
  namespace lexer {

    int tokenize(basic_istream<char>* is, vector<unique_ptr<Token>>* tokens) {
      char c;

      while (is && is->get(c)) {
        if (isspace(c))
          continue;

        if (isdigit(c)) { // Number: [0-9]+
          string number;
          number += c;

          while (is && is->get(c) && isdigit(c))
            number += c;

          double value = strtod(number.c_str(), nullptr);
          auto numberToken = llvm::make_unique<NumberToken>(value);
          tokens->push_back(move(numberToken));

        } else { // other token
          string str;
          str += c;

          while (is && is->get(c) && !isspace(c))
            str += c;

          auto otherToken = llvm::make_unique<OtherToken>(str);
          tokens->push_back(move(otherToken));
        }
      }

      return 0;
    }

  }
}
