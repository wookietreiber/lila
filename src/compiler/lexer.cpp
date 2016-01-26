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

          bool hasdot = false;

          while (is && is->get(c)) {
            if (isdigit(c))
              number += c;
            else if (c == '.' && !hasdot) {
              number += c;
              hasdot = true;
            } else
              break;
          }

          double value = strtod(number.c_str(), nullptr);
          auto numberToken = llvm::make_unique<NumberToken>(value);
          tokens->push_back(move(numberToken));

          // check if last char of number is a dot ...
          if (number.back() == '.') {
            // ... to add final dot to tokens to allow for:
            //   number.method, e.g.:
            //     5.abs
            //   number.method(args), e.g.:
            //     5.max(6)
            auto dotToken = llvm::make_unique<OtherToken>(".");
            tokens->push_back(move(dotToken));
          }

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
