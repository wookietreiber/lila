/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#ifndef LILA_LEXER_H
#define LILA_LEXER_H

#include <iostream>
#include <memory>
#include <vector>

using namespace std;

namespace lila {
  namespace lexer {

    class Token {
    public:
      virtual string toString() = 0;
      virtual int foo () = 0;
    };

    class NumberToken : public Token {
      double value;

    public:
      NumberToken(double value) : value(value) {}
      int foo () { return value; }
      string toString() {
        return to_string(value);
      }
    };

    class OtherToken : public Token {
      string value;

    public:
      OtherToken(string value) : value(value) {}
      int foo () { return 2; }
      string toString() {
        return value;
      }
    };

    int tokenize(basic_istream<char>* is, vector<unique_ptr<Token>>* tokens);

  }
}

#endif
