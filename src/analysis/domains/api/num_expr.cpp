/*
 * num_expr.cpp
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/num_expr.h>

std::ostream &analysis::api::operator <<(std::ostream &out, num_expr &_this) {
  _this.put(out);
  return out;
}

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

void analysis::api::num_expr_lin::put(std::ostream &out) {
  out << inner;
}

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
