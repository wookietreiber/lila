/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#ifndef LILA_PARSER_H
#define LILA_PARSER_H

#include <map>

#include "ast.hpp"
#include "lexer.hpp"

using namespace lila::ast;
using namespace lila::lexer;

namespace lila {
  namespace parser {

    class ParserResult {
    public:
      virtual ~ParserResult() {}
    };

    class ParserSuccess : public ParserResult {
    public:
      unique_ptr<ASTNode> ast;
      explicit ParserSuccess(unique_ptr<ASTNode> ast) : ast(move(ast)) {}
    };

    class ParserFailure : public ParserResult {
    public:
      string msg;
      explicit ParserFailure(string msg) : msg(msg) {}
    };

    class ParserValue {
    public:
      string name;
      vector<string> args;
      explicit ParserValue(string name, vector<string> args)
        : name(name), args(args) {}
    };

    class Parser {
    private:
      unsigned int anonindex = 0;
      vector<unique_ptr<Token> >* tokens;
      map<string, int> operatorPrecendences;
      Token * curtok;
      vector<unique_ptr<Token> >::size_type pos = 0;
      string error;
      int indent = 0;

      int getPrecedence(string op) {
        int precedence = operatorPrecendences[op];

        if (precedence <= 0)
          return 0;

        return precedence;
      }

      int nextToken() {
        auto size = tokens->size();

        if (pos >= size) {
          curtok = NULL;
          return 0;
        }

        curtok = tokens->at(pos++).get();

        return 1;
      }

      vector<string> curscope;
      map<string, vector<ParserValue> > scoped_values;

      void removeScope() {
        curscope.pop_back();
      }

      string scopestr() {
        ostringstream oss;
        auto size = curscope.size();
        for (unsigned i = 0; i < size; i++) {
          oss << curscope[i];
          if (i < (size - 1)) oss << '.';
        }
        return oss.str();
      }

      void addScope(string level) {
        // first add to curscope
        curscope.push_back(level);

        // second add empty values to scope map
        vector<ParserValue > values;
        scoped_values[scopestr()] = values;
      }

      void addScopedValue(string name, vector<string> args) {
        string scope = scopestr();

        ParserValue value(name, args);

        vector<ParserValue> values = scoped_values[scope];
        values.push_back(value);

        scoped_values[scope] = values;
      }

      bool existsScopedValue(string name) {
        // search from current scope outwards
        for (int i = curscope.size(); i >= 0; i--) {
          ostringstream oss;
          for (int j = 0; j < i; j++) {
            oss << curscope[j];
            if (j < (i - 1)) oss << '.';
          }

          string scope = oss.str();

          vector<ParserValue> values = scoped_values[scope];

          for (auto it = values.begin() ; it != values.end(); ++it)
            if (name == it->name) return true;
        }

        return false;
      }

      bool existsCurrentScope(string name) {
        vector<ParserValue> values = scoped_values[scopestr()];

        for (auto it = values.begin() ; it != values.end(); ++it)
          if (name == it->name) return true;

        return false;
      }

      unique_ptr<ExprAST> parseExpression();
      unique_ptr<ExprAST> parseNumberExpr(NumberToken* tok);
      unique_ptr<ExprAST> parseParenExpr();
      unique_ptr<ExprAST> parsePrimary();
      unique_ptr<ExprAST> parseBinOpRHS(unique_ptr<ExprAST> lhs, int prec);
      unique_ptr<ExprAST> parseIdentifier(string name);
      unique_ptr<ExprAST> parseBlock();
      unique_ptr<ExprAST> parseTopLevelBlock();
      unique_ptr<ValueAST> parseValue();
      unique_ptr<DefAST> parseDef();

    public:
      explicit Parser(vector<unique_ptr<Token> >* tokens) : tokens(tokens) {
        operatorPrecendences["+"] = 20;
        operatorPrecendences["-"] = 20;
        operatorPrecendences["*"] = 40;
      }

      unique_ptr<ParserResult> parse();
    };

  }
}

#endif
