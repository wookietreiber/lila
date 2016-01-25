/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
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

    class Parser {
    private:
      vector<unique_ptr<Token>>* tokens;
      map<string, int> operatorPrecendences;
      Token * curtok;
      vector<unique_ptr<Token>>::size_type pos = 0;

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

      unique_ptr<ExprAST> parseExpression();
      unique_ptr<ExprAST> parseNumberExpr(NumberToken* tok);
      unique_ptr<ExprAST> parsePrimary();
      unique_ptr<ExprAST> parseBinOpRHS(unique_ptr<ExprAST> lhs, int prec);

    public:
      explicit Parser(vector<unique_ptr<Token>>* tokens) : tokens(tokens) {
        operatorPrecendences["+"] = 20;
        operatorPrecendences["-"] = 20;
        operatorPrecendences["*"] = 40;
      }

      unique_ptr<ASTNode> parse();
    };

  }
}

#endif
