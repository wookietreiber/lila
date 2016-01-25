/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#ifndef LILA_AST_H
#define LILA_AST_H

#include <memory>

using namespace std;

namespace lila {
  namespace ast {

    class ASTNode {
    public:
      virtual string toString() = 0;
      virtual ~ASTNode() {}
    };

    class ExprAST : public ASTNode {
    };

    class NumberExprAST : public ExprAST {

    public:
      double Val;
      explicit NumberExprAST(double Val) : Val(Val) {}
      string toString() {
        return to_string(Val);
      }
    };

    class BinaryExprAST : public ExprAST {

    public:
      string Op;
      unique_ptr<ExprAST> LHS, RHS;
      BinaryExprAST(string Op, unique_ptr<ExprAST> LHS, unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(move(LHS)), RHS(move(RHS)) {}
      string toString() {
        return '(' + LHS->toString() + ' ' + Op + ' ' + RHS->toString() + ')';
      }
    };

  }
}

#endif
