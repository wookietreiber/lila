/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
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
      explicit NumberToken(const double &value) : value(value) {}
      string toString() {
        return to_string(value);
      }
    };

    class OtherToken : public Token {
    public:
      string name;
      explicit OtherToken(const string &name) : name(name) {}
      string toString() {
        return name;
      }
    };

    class BlockClose : public Token {
    public:
      explicit BlockClose() {}
      string toString() {
        return "}";
      }
    };

    class BlockOpen : public Token {
    public:
      explicit BlockOpen() {}
      string toString() {
        return "{";
      }
    };

    class ParenClose : public Token {
    public:
      explicit ParenClose() {}
      string toString() {
        return ")";
      }
    };

    class ParenOpen : public Token {
    public:
      explicit ParenOpen() {}
      string toString() {
        return "(";
      }
    };

    class CommaToken : public Token {
    public:
      explicit CommaToken() {}
      string toString() {
        return ",";
      }
    };

    class ValueToken : public Token {
    public:
      explicit ValueToken() {}
      string toString() {
        return "val";
      }
    };

    class DefToken : public Token {
    public:
      explicit DefToken() {}
      string toString() {
        return "def";
      }
    };

    class AssignmentToken : public Token {
    public:
      explicit AssignmentToken() {}
      string toString() {
        return "=";
      }
    };

    class NewlineToken : public Token {
    public:
      explicit NewlineToken() {}
      string toString() {
        return "\\n";
      }
    };

  }
}

#endif
