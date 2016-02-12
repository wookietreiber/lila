/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include "lexer.hpp"

namespace lila {
  namespace lexer {

    unique_ptr<LexerResult> tokenize(basic_istream<char>* is) {
      auto tokens = llvm::make_unique<vector<unique_ptr<Token> > >();

      unsigned int parens = 0;
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

          if (str.compare("def") == 0) {
            auto token = llvm::make_unique<DefToken>();
            tokens->push_back(move(token));
          } else if (str.compare("val") == 0) {
            auto valueToken = llvm::make_unique<ValueToken>();
            tokens->push_back(move(valueToken));
          } else {
            auto otherToken = llvm::make_unique<OtherToken>(str);
            tokens->push_back(move(otherToken));
          }

        } else if (c == '(') {
          parens++;
          auto parenopen = llvm::make_unique<ParenOpen>();
          tokens->push_back(move(parenopen));
          is->get(c);

        } else if (c == ')') {
          if (parens == 0) {
            auto failure = llvm::make_unique<LexerFailure>("closing parens when none is open");
            return move(failure);
          } else {
            parens--;
            auto parenclose = llvm::make_unique<ParenClose>();
            tokens->push_back(move(parenclose));
            is->get(c);
          }

        } else if (ispunct(c)) { // punctuation token
          string str;
          str += c;

          while (is && is->get(c) && ispunct(c))
            str += c;

          if (str.compare("=") == 0) {
            auto assignmentToken = llvm::make_unique<AssignmentToken>();
            tokens->push_back(move(assignmentToken));
          } else {
            auto otherToken = llvm::make_unique<OtherToken>(str);
            tokens->push_back(move(otherToken));
          }

        } else if (c == '\n') {
          auto token = llvm::make_unique<NewlineToken>();
          tokens->push_back(move(token));
          is->get(c);

        } else { // ignore
          is->get(c);
        }
      }

      auto success = llvm::make_unique<LexerSuccess>(move(tokens));
      return move(success);
    }

  }
}
