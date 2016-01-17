/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include "parser.hpp"

namespace lila {
  namespace parser {

    unique_ptr<ExprAST> Parser::parseNumberExpr(NumberToken* tok) {
      double number = tok->value;
      auto numberast = llvm::make_unique<NumberExprAST>(number);
      return move(numberast);
    }

    unique_ptr<ExprAST> Parser::parseBinOpRHS(unique_ptr<ExprAST> lhs, int prec) {
      while (curtok) {
        // expecting some op token
        if (auto op = dynamic_cast<OtherToken*>(curtok.get())) {
          int opprec = getPrecedence(op->value);

          if (opprec < prec)
            return lhs;

          if (!nextToken()) {
            throw "expected token after operation";
          }

          // parse primary expression after the operator
          auto rhs = parsePrimary();
          if (!rhs)
            return nullptr;

          nextToken();

          if (!curtok)
            return llvm::make_unique<BinaryExprAST>(op->value, move(lhs), move(rhs));

          // if op binds less tightly with rhs than op after rhs, let
          // the pending op take rhs as its lhs
          if (auto nextop = dynamic_cast<OtherToken*>(curtok.get())) {
            int nextprec = getPrecedence(nextop->value);

            if (opprec < nextprec) {
              rhs = parseBinOpRHS(move(rhs), opprec + 1);

              if (!rhs)
                return nullptr;
            }

            // Merge lhs/rhs.
            lhs = llvm::make_unique<BinaryExprAST>(op->value, move(lhs), move(rhs));
          }

        } else {
          throw "unknown token when expecting an operation";
        }
      }

      return lhs;
    }

    unique_ptr<ExprAST> Parser::parsePrimary() {
      if (auto t = dynamic_cast<NumberToken*>(curtok.get())) {
        return parseNumberExpr(t);
      } else {
        throw "unknown token when expecting an expression";
      }
    }

    unique_ptr<ExprAST> Parser::parseExpression() {
      auto lhs = parsePrimary();
      nextToken();

      if (!lhs)
        return nullptr;

      return parseBinOpRHS(move(lhs), 0);
    }

    unique_ptr<ASTNode> Parser::parse() {
      unique_ptr<ASTNode> curast;
      pos = 0;

      while (nextToken()) {
        curast = parseExpression();
      }

      return curast;
    }
  }
}
