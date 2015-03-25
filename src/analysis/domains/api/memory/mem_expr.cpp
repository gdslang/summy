/*
 * mem_expr.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/memory/mem_expr.h>
#include <summy/analysis/domains/api/memory/mem_visitor.h>

using namespace std;
using namespace analysis::api;

/*
 * num_expr
 */

std::ostream &analysis::api::operator <<(ostream &out, mem_expr &_this) {
  _this.put(out);
  return out;
}

/*
 * num_expr_re
 */

void analysis::api::mem_expr_re::put(std::ostream &out) {
  out << *expr;
}

analysis::api::mem_expr_re::~mem_expr_re() {
}

void analysis::api::mem_expr_re::accept(mem_visitor &v) {
  v.visit(this);
}

/*
 * num_expr_deref
 */

void analysis::api::mem_expr_deref::put(std::ostream &out) {
  out << "*" << *inner;
}

analysis::api::mem_expr_deref::~mem_expr_deref() {
}

void analysis::api::mem_expr_deref::accept(mem_visitor &v) {
  v.visit(this);
}
