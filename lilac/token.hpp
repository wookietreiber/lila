/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#ifndef LILA_TOKEN_H
#define LILA_TOKEN_H

using namespace std;

namespace lila {
  namespace token {

    class Token {
    public:
      virtual string toString() = 0;
    };

    class NumberToken : public Token {
    public:
      double value;
      explicit NumberToken(double value) : value(value) {}
      string toString() {
        return to_string(value);
      }
    };

    class OtherToken : public Token {
    public:
      string value;
      explicit OtherToken(string value) : value(value) {}
      string toString() {
        return value;
      }
    };

  }
}

#endif
