/*
 * num_expr.h
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/api/numeric/num_linear.h>
#include <summy/analysis/domains/api/numeric/num_visitor.h>

namespace analysis {
namespace api {

class num_expr {
private:
  virtual void put(std::ostream &out) = 0;

public:
  virtual ~num_expr() {
  }

  virtual void accept(num_visitor &v) = 0;
  friend std::ostream &operator<<(std::ostream &out, num_expr &_this);
};

std::ostream &operator<<(std::ostream &out, num_expr &_this);

enum num_cmp_op {
  LE, LT, GE, GT, EQ, NEQ
};

class num_expr_cmp: public num_expr {
private:
  num_linear *opnd;
  num_cmp_op op;

  virtual void put(std::ostream &out);
public:
  num_expr_cmp(num_linear *opnd, num_cmp_op op) :
      opnd(opnd), op(op) {
  }
  ~num_expr_cmp();

  num_linear *get_opnd() {
    return opnd;
  }

  num_cmp_op get_op() {
    return op;
  }

  num_expr_cmp *negate() const;
  num_expr_cmp *copy() const;

  void accept(num_visitor &v);
};

class num_expr_lin: public num_expr {
private:
  num_linear *inner;

  virtual void put(std::ostream &out);
public:
  num_expr_lin(num_linear *inner) :
      inner(inner) {
  }
  ~num_expr_lin();

  void accept(num_visitor &v);

  num_linear *get_inner() {
    return inner;
  }
};

enum num_bin {
  MUL, DIV, MOD, SHL, SHR, SHRS, AND, OR, XOR
};

class num_expr_bin: public num_expr {
private:
  num_linear *opnd1;
  num_bin op;
  num_linear *opnd2;

  virtual void put(std::ostream &out);
public:
  num_expr_bin(num_linear *opnd1, num_bin op, num_linear *opnd2) :
      opnd1(opnd1), op(op), opnd2(opnd2) {
  }
  ~num_expr_bin();

  void accept(num_visitor &v);

  num_linear *get_opnd1() {
    return opnd1;
  }

  num_bin get_op() const {
    return op;
  }

  num_linear *get_opnd2() {
    return opnd2;
  }
};

}
}
