/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include "parser.hpp"

namespace lila {
  namespace parser {

    unique_ptr<ExprAST> Parser::parseNumberExpr(NumberToken* tok) {
      double number = tok->value;
      nextToken();
      auto numberast = llvm::make_unique<NumberExprAST>(number);
      return move(numberast);
    }

    unique_ptr<ExprAST> Parser::parseBinOpRHS(unique_ptr<ExprAST> lhs, int prec) {
      while (curtok) {
        // expecting some op token
        if (auto op = dynamic_cast<OtherToken*>(curtok)) {
          int opprec = getPrecedence(op->name);

          if (opprec < prec)
            return lhs;

          if (!nextToken()) {
            error = "expected token after operation";
            return nullptr;
          }

          // parse primary expression after the operator
          auto rhs = parsePrimary();
          if (!rhs)
            return nullptr;

          if (!curtok)
            return llvm::make_unique<BinaryExprAST>(op->name, move(lhs), move(rhs));

          // if op binds less tightly with rhs than op after rhs, let
          // the pending op take rhs as its lhs
          if (auto nextop = dynamic_cast<OtherToken*>(curtok)) {
            int nextprec = getPrecedence(nextop->name);

            if (opprec < nextprec) {
              rhs = parseBinOpRHS(move(rhs), opprec + 1);

              if (!rhs)
                return nullptr;
            }

            // Merge lhs/rhs.
            lhs = llvm::make_unique<BinaryExprAST>(op->name, move(lhs), move(rhs));
          } else if (dynamic_cast<CommaToken*>(curtok)) {
            return llvm::make_unique<BinaryExprAST>(op->name, move(lhs), move(rhs));
          } else if (dynamic_cast<NewlineToken*>(curtok)) {
            return llvm::make_unique<BinaryExprAST>(op->name, move(lhs), move(rhs));
          } else if (dynamic_cast<ParenClose*>(curtok)) {
            return llvm::make_unique<BinaryExprAST>(op->name, move(lhs), move(rhs));
          }

        } else if (dynamic_cast<CommaToken*>(curtok)) {
          return lhs;
        } else if (dynamic_cast<NewlineToken*>(curtok)) {
          return lhs;
        } else if (dynamic_cast<ParenClose*>(curtok)) {
          return lhs;
        } else {
          error = "unknown token \"" + curtok->toString() + "\" when expecting an operation";
          return nullptr;
        }
      }

      return lhs;
    }

    unique_ptr<ExprAST> Parser::parseParenExpr() {
      nextToken(); // eat (
      auto expr = parseExpression();
      if (!expr) {
        return nullptr;
      }

      if (dynamic_cast<ParenClose*>(curtok)) {
        nextToken(); // eat )
        return expr;
      } else {
        error = "expected ')'";
        return nullptr;
      }
    }

    unique_ptr<ExprAST> Parser::parsePrimary() {
      if (auto t = dynamic_cast<NumberToken*>(curtok)) {
        return parseNumberExpr(t);
      } else if (dynamic_cast<ParenOpen*>(curtok)) {
        return parseParenExpr();
      } else if (auto t = dynamic_cast<OtherToken*>(curtok)) {
        if (names.find(t->name) != names.end()) {
          return parseIdentifier(t->name);
        } else {
          error = "unknown token \"" + curtok->toString() + "\" when expecting primary";
          return nullptr;
        }
      } else {
        error = "unknown token \"" + curtok->toString() + "\" when expecting primary";
        return nullptr;
      }
    }

    unique_ptr<ExprAST> Parser::parseIdentifier(string name) {
      auto args = llvm::make_unique<vector<unique_ptr<ExprAST> > >();

      nextToken();

      if (dynamic_cast<ParenOpen*>(curtok)) {
        nextToken(); // eat "(" token
        bool expectarg = true;

        while (curtok) {
          if (dynamic_cast<ParenClose*>(curtok)) {
            if (expectarg) {
              error = "expecting an argument";
              return nullptr;
            } else {
              break;
            }
          } else if (dynamic_cast<CommaToken*>(curtok)) {
            nextToken();
            if (expectarg) {
              error = "expecting an argument";
              return nullptr;
            } else {
              expectarg = true;
            }
          } else {
            if (expectarg) {
              auto expr = parseExpression();
              if (!expr) {
                error = "expected expression as argument: " + error;
                return nullptr;
              }
              args->push_back(move(expr));
              expectarg = false;
            } else {
              error = "didn't expect another argument";
              return nullptr;
            }
          }
        }

        nextToken(); // eat ")" token
      }

      auto ast = llvm::make_unique<CallAST>(name, move(args));
      return move(ast);
    }

    unique_ptr<ExprAST> Parser::parseExpression() {
      auto lhs = parsePrimary();

      if (!lhs)
        return nullptr;

      return parseBinOpRHS(move(lhs), 0);
    }

    // val name = expr
    unique_ptr<ValueAST> Parser::parseValue() {
      string name;

      nextToken(); // eat "val" token

      if (auto t = dynamic_cast<OtherToken*>(curtok)) {
        name = t->name;
      } else {
        error = "expected value name";
        return nullptr;
      }

      nextToken(); // eat name of value token

      if (!dynamic_cast<AssignmentToken*>(curtok)) {
        error = "expected \"=\"";
        return nullptr;
      }

      nextToken(); // eat "=" token

      auto expr = parseExpression();

      if (!expr) {
        error = "expected expression: " + error;
        return nullptr;
      }

      if (names.find(name) != names.end()) {
        error = name + " is already defined";
        return nullptr;
      }

      vector<string> emptyargs;
      names[name] = emptyargs;
      auto ast = llvm::make_unique<ValueAST>(name, move(expr));
      return ast;
    }

    // def name = expr
    unique_ptr<DefAST> Parser::parseDef() {
      string name;
      vector<string> args;

      nextToken(); // eat "def" token

      if (auto t = dynamic_cast<OtherToken*>(curtok)) {
        name = t->name;

        if (names.find(name) != names.end()) {
          error = name + " is already defined";
          return nullptr;
        }
      } else {
        error = "expected def name";
        return nullptr;
      }

      nextToken(); // eat name of value token

      if (dynamic_cast<ParenOpen*>(curtok)) {
        nextToken(); // eat "(" token
        bool expectarg = true;

        while (curtok) {
          if (dynamic_cast<ParenClose*>(curtok)) {
            if (expectarg) {
              error = "expecting an argument";
              return nullptr;
            } else {
              break;
            }
          } else if (auto t = dynamic_cast<OtherToken*>(curtok)) {
            if (expectarg) {
              string name = t->name;

              if (names.find(name) != names.end()) {
                error = name + " is already defined";
                return nullptr;
              }

              vector<string> emptyargs;
              names[name] = emptyargs;

              args.push_back(name);
              expectarg = false;
            } else {
              error = "didn't expect another argument";
              return nullptr;
            }
          } else if (dynamic_cast<CommaToken*>(curtok)) {
            if (expectarg) {
              error = "expecting an argument";
              return nullptr;
            } else {
              expectarg = true;
            }
          } else {
            error = "expected arguments or end of arguments, i.e. \")\"";
            return nullptr;
          }

          nextToken();
        }

        nextToken(); // eat ")" token
      }

      if (!dynamic_cast<AssignmentToken*>(curtok)) {
        error = "expected \"=\"";
        return nullptr;
      }

      nextToken(); // eat "=" token

      auto expr = parseExpression();

      if (!expr) {
        error = "expected expression: " + error;
        return nullptr;
      }

      if (names.find(name) != names.end()) {
        error = name + " is already defined";
        return nullptr;
      }

      names[name] = args;
      auto ast = llvm::make_unique<DefAST>(name, args, move(expr));
      return ast;
    }

    unique_ptr<ExprAST> Parser::parseTopLevelBlock(unique_ptr<ASTNode> first) {
      auto body = llvm::make_unique<vector<unique_ptr<ASTNode> > >();
      body->push_back(move(first));

      unique_ptr<ASTNode> curast;

      while (nextToken()) {
        if (dynamic_cast<NewlineToken*>(curtok)) {
          continue;
        } else if (dynamic_cast<ValueToken*>(curtok)) {
          curast = parseValue();
        } else {
          curast = parseExpression();
        }

        if (curast) {
          body->push_back(move(curast));
        } else {
          return nullptr;
        }
      }

      if (!dynamic_cast<ExprAST*>(body->back().get())) {
        error = "top level block does not end in expression";
        return nullptr;
      }

      auto block = llvm::make_unique<BlockAST>(move(body));
      return move(block);
    }

    unique_ptr<ParserResult> Parser::parse() {
      unique_ptr<ASTNode> curast;
      pos = 0;

      while (nextToken()) {
        if (dynamic_cast<NewlineToken*>(curtok)) {
          continue;
        } else if (dynamic_cast<DefToken*>(curtok)) {
          auto ast = parseDef();
          if (!ast) {
            auto failure = llvm::make_unique<ParserFailure>(error);
            return move(failure);
          }
          curast = parseTopLevelBlock(move(ast));
        } else if (dynamic_cast<ValueToken*>(curtok)) {
          auto ast = parseValue();
          curast = parseTopLevelBlock(move(ast));
        } else {
          auto ast = parseExpression();
          curast = parseTopLevelBlock(move(ast));
        }

        if (!curast) {
          auto failure = llvm::make_unique<ParserFailure>(error);
          return move(failure);
        }
      }

      auto success = llvm::make_unique<ParserSuccess>(move(curast));
      return move(success);
    }
  }
}
