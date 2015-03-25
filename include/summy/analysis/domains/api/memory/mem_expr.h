/*
 * mem_expr.h
 *
 *  Created on: Mar 25, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <cppgdsl/rreil/expr/expr.h>
#include <iostream>

namespace analysis {
namespace api {

class mem_visitor;

class mem_expr {
private:
  virtual void put(std::ostream &out) = 0;

public:
  virtual ~mem_expr() {
  }

  virtual void accept(mem_visitor &v) = 0;
  friend std::ostream &operator<<(std::ostream &out, mem_expr &_this);
};

std::ostream &operator<<(std::ostream &out, mem_expr &_this);

class mem_expr_re: public mem_expr {
private:
  gdsl::rreil::expr *expr;

  virtual void put(std::ostream &out);
public:
  mem_expr_re(gdsl::rreil::expr *expr) :
    expr(expr) {
  }
  ~mem_expr_re();

  void accept(mem_visitor &v);
};

class mem_expr_deref: public mem_expr {
private:
  mem_expr *inner;

  virtual void put(std::ostream &out);
public:
  mem_expr_deref(mem_expr *inner) :
    inner(inner) {
  }
  ~mem_expr_deref();

  void accept(mem_visitor &v);
};

}
}
