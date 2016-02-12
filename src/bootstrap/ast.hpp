/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#ifndef LILA_AST_H
#define LILA_AST_H

#include <memory>

#include <sstream>
#include <vector>

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
      explicit BinaryExprAST(string Op, unique_ptr<ExprAST> LHS, unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(move(LHS)), RHS(move(RHS)) {}
      string toString() {
        return '(' + LHS->toString() + ' ' + Op + ' ' + RHS->toString() + ')';
      }
    };

    class ValueAST : public ASTNode {
    public:
      string name;
      unique_ptr<ExprAST> expr;
      explicit ValueAST(string name, unique_ptr<ExprAST> expr) : name(name), expr(move(expr)) {}
      string toString() {
        return "val " + name + " = " + expr->toString();
      }
    };

    class DefAST : public ASTNode {
    public:
      string name;
      unique_ptr<ExprAST> body;
      explicit DefAST(string name, unique_ptr<ExprAST> body) : name(name), body(move(body)) {}
      string toString() {
        return "def " + name + " = " + body->toString();
      }
    };

    class CallAST : public ExprAST {
    public:
      string name;
      explicit CallAST(string name) : name(name) {}
      string toString() {
        return name;
      }
    };

    class BlockAST : public ExprAST {
    public:
      unique_ptr<vector<unique_ptr<ASTNode> > > body;
      explicit BlockAST(unique_ptr<vector<unique_ptr<ASTNode> > > body) : body(move(body)) {}
      string toString() {
        ostringstream oss;
        oss << "{" << endl;
        for (auto it = body->begin() ; it != body->end(); ++it) {
          ASTNode * ast = it->get();
          oss << "  " << ast->toString() << endl;
        }
        oss << "}" << endl;
        return oss.str();
      }
    };

  }
}

#endif
