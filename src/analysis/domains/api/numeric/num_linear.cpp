/*
 * num_linear.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/numeric/num_linear.h>
#include <summy/value_set/vs_finite.h>

using namespace summy;
using namespace analysis::api;


/*
 * num_linear
 */

std::ostream &analysis::api::operator <<(std::ostream &out, num_linear &_this) {
  _this.put(out);
  return out;
}

/*
 * num_linear_term
 */

void analysis::api::num_linear_term::put(std::ostream &out) {
  if(scale != 1) out << scale << "*";
  out << *var << " + " << *next;
}

analysis::api::num_linear_term::num_linear_term(int64_t scale, num_var *var) :
    scale(scale), var(var) {
  next = new num_linear_vs(vs_finite::zero);
}

analysis::api::num_linear_term::~num_linear_term() {
  delete var;
  delete next;
}

num_linear_term *analysis::api::num_linear_term::negate() {
  return new num_linear_term(-scale, var->copy(), next->negate());
}

void analysis::api::num_linear_term::accept(num_visitor &v) {
  v.visit(this);
}

/*
 * num_linear_vs
 */

void analysis::api::num_linear_vs::put(std::ostream &out) {
  out << *value_set;
}

num_linear_vs *analysis::api::num_linear_vs::negate() {
  return new num_linear_vs(-*value_set);
}

void analysis::api::num_linear_vs::accept(num_visitor &v) {
  v.visit(this);
}
