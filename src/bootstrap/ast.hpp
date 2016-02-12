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
      double value;
      explicit NumberExprAST(const double &value) : value(value) {}
      string toString() {
        return to_string(value);
      }
    };

    class BinaryExprAST : public ExprAST {

    public:
      string op;
      unique_ptr<ExprAST> lhs, rhs;
      explicit BinaryExprAST(const string &op, unique_ptr<ExprAST> lhs, unique_ptr<ExprAST> rhs)
        : op(op), lhs(move(lhs)), rhs(move(rhs)) {}
      string toString() {
        return '(' + lhs->toString() + ' ' + op + ' ' + rhs->toString() + ')';
      }
    };

    class ValueAST : public ASTNode {
    public:
      string name;
      unique_ptr<ExprAST> expr;
      explicit ValueAST(const string &name, unique_ptr<ExprAST> expr)
        : name(name), expr(move(expr)) {}
      string toString() {
        return "val " + name + " = " + expr->toString();
      }
    };

    class DefAST : public ASTNode {
    public:
      string name;
      unique_ptr<ExprAST> body;
      explicit DefAST(const string &name, unique_ptr<ExprAST> body)
        : name(name), body(move(body)) {}
      string toString() {
        return "def " + name + " = " + body->toString();
      }
    };

    class CallAST : public ExprAST {
    public:
      string name;
      explicit CallAST(const string &name) : name(name) {}
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
        oss << '{' << endl;
        for (auto it = body->begin() ; it != body->end(); ++it) {
          ASTNode * ast = it->get();
          oss << "  " << ast->toString() << endl;
        }
        oss << '}' << endl;
        return oss.str();
      }
    };

  }
}

#endif
