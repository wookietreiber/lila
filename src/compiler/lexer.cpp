/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include "lexer.hpp"

namespace lila {
  namespace lexer {

    void tokenize(basic_istream<char>* is, vector<unique_ptr<Token>>* tokens) {
      char c;
      is->get(c);

      while (is && !is->eof()) {
        if (isdigit(c)) { // number: [0-9]+
          string number;
          number += c;

          while (is && is->get(c) && isdigit(c))
            number += c;

          double value = strtod(number.c_str(), nullptr);
          auto numberToken = llvm::make_unique<NumberToken>(value);
          tokens->push_back(move(numberToken));

        } else if (isalpha(c)) { // [a-zA-Z][a-zA-Z0-9]* token
          string str;
          str += c;

          while (is && is->get(c) && isalnum(c))
            str += c;

          auto otherToken = llvm::make_unique<OtherToken>(str);
          tokens->push_back(move(otherToken));

        } else if (ispunct(c)) { // punctuation token
          string str;
          str += c;

          while (is && is->get(c) && ispunct(c))
            str += c;

          auto otherToken = llvm::make_unique<OtherToken>(str);
          tokens->push_back(move(otherToken));

        } else { // ignore
          is->get(c);
        }
      }
    }

  }
}
