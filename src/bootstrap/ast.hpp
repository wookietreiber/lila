/*            __ __                                                     *\
**     __    /_// /  ___            lila bootstrap compiler             **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#ifndef LILA_AST_H
#define LILA_AST_H

#include <memory>

#include <sstream>
#include <vector>

#include "util.hpp"

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
      vector<string> args;
      unique_ptr<ExprAST> body;
      explicit DefAST(const string &name, const vector<string> &args, unique_ptr<ExprAST> body)
        : name(name), args(args), body(move(body)) {}
      string toString() {
        ostringstream oss;

        oss << "def " << name;

        if (!args.empty()) {
          oss << '(' << util::mkString(args, ", ") << ')';
        }

        oss << " = " << body->toString();

        return oss.str();
      }
    };

    class CallAST : public ExprAST {
    public:
      string name;
      unique_ptr<vector<unique_ptr<ExprAST> > > args;
      explicit CallAST(const string &name, unique_ptr<vector<unique_ptr<ExprAST> > > args)
        : name(name), args(move(args)) {}
      string toString() {
        ostringstream oss;
        oss << name;
        if (!args->empty()) {
          oss << '(';
          auto size = args->size();
          for (unsigned i = 0; i < size; i++) {
            oss << args->at(i)->toString();
            if (i < (size - 1)) oss << ", ";
          }
          oss << ')';
        }
        return oss.str();
      }
    };

    class BlockAST : public ExprAST {
    public:
      unique_ptr<vector<unique_ptr<ASTNode> > > body;
      int indent;
      explicit BlockAST(unique_ptr<vector<unique_ptr<ASTNode> > > body, const int i) : body(move(body)) {
        indent = i;
      }
      string toString() {
        ostringstream oss;
        oss << '{' << endl;
        for (auto it = body->begin() ; it != body->end(); ++it) {
          ASTNode * ast = it->get();
          for (int i = 0; i < indent; i++)
            oss << "  ";
          oss << ast->toString() << endl;
        }
        for (int i = 0; i < indent - 1; i++)
          oss << "  ";
        oss << '}' << endl;
        return oss.str();
      }
    };

  }
}

#endif
