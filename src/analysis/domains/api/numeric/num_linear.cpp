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

num_linear_term *analysis::api::num_linear_term::negate() const {
  return new num_linear_term(-scale, var->copy(), next->negate());
}

num_linear_term *analysis::api::num_linear_term::copy() const {
  return new num_linear_term(scale, var->copy(), next->copy());
}

void analysis::api::num_linear_term::accept(num_visitor &v) {
  v.visit(this);
}

bool analysis::api::num_linear_term::operator==(num_linear &b) {
  bool equals = false;
  num_visitor nv;
  nv._([&](num_linear_term *nt_b) {
    equals = scale == nt_b->scale && *var == *nt_b->var && *next == *nt_b->next;
  });
  b.accept(nv);
  return equals;
}

/*
 * num_linear_vs
 */

void analysis::api::num_linear_vs::put(std::ostream &out) {
  out << *value_set;
}

num_linear_vs *analysis::api::num_linear_vs::negate() const {
  return new num_linear_vs(-*value_set);
}

void analysis::api::num_linear_vs::accept(num_visitor &v) {
  v.visit(this);
}

num_linear_vs *analysis::api::num_linear_vs::copy() const {
  return new num_linear_vs(value_set);
}

bool analysis::api::num_linear_vs::operator==(num_linear &b) {
  bool equals = false;
  num_visitor nv;
  nv._([&](num_linear_vs *lvs_b) {
    equals = *value_set == lvs_b->value_set;
  });
  b.accept(nv);
  return equals;
}
