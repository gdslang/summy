/*
 * num_expr.cpp
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/num_expr.h>

/*
 * num_expr
 */

std::ostream &analysis::api::operator <<(std::ostream &out, num_expr &_this) {
  _this.put(out);
  return out;
}

/*
 * num_exr_cmp
 */

void analysis::api::num_expr_cmp::put(std::ostream &out) {
  out << *opnd << " ";
  switch(op) {
    case LE: {
      out << "<=";
      break;
    }
    case GE: {
      out << ">=";
      break;
    }
    case EQ: {
      out << "==";
      break;
    }
    case NEQ: {
      out << "!=";
      break;
    }
  }
  out << " 0";
}

analysis::api::num_expr_cmp::~num_expr_cmp() {
  delete opnd;
}

void analysis::api::num_expr_cmp::accept(num_visitor &v) {
  v.visit(this);
}

/*
 * num_expr_lin
 */

void analysis::api::num_expr_lin::put(std::ostream &out) {
  out << inner;
}

analysis::api::num_expr_lin::~num_expr_lin() {
  delete inner;
}

void analysis::api::num_expr_lin::accept(num_visitor &v) {
  v.visit(this);
}

/*
 * num_expr_bin
 */

void analysis::api::num_expr_bin::put(std::ostream &out) {
  out << *opnd1 << " ";
  switch(op) {
    case MUL: {
      out << "*";
      break;
    }
    case DIV: {
      out << "/";
      break;
    }
    case MOD: {
      out << "%";
      break;
    }
    case SHL: {
      out << "<<";
      break;
    }
    case SHR: {
      out << ">>";
      break;
    }
    case AND: {
      out << "&";
      break;
    }
    case OR: {
      out << "&";
      break;
    }
    case XOR: {
      out << "&";
      break;
    }
  }
  out << " " << *opnd2;
}

analysis::api::num_expr_bin::~num_expr_bin() {
  delete opnd1;
  delete opnd2;
}

void analysis::api::num_expr_bin::accept(num_visitor &v) {
  v.visit(this);
}
