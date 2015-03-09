/*
 * numeric_api_builder.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: Julian Kranz
 */
#include <summy/test/analysis/domains/numeric_api_builder.h>
#include <summy/value_set/vs_finite.h>

using namespace summy;
using namespace analysis::api;

nap_lin::nap_lin(id_shared_t id) {
  num_var *var = new num_var(id);
  lin = new num_linear_term(var);
}

nap_lin::nap_lin(summy::vs_shared_t vs) {
  lin = new num_linear_vs(vs);
}

nap_lin::nap_lin(int64_t val) {
  lin = new num_linear_vs(vs_shared_t(new vs_finite(val)));
}

analysis::api::num_expr *nap_lin::expr() {
  return new num_expr_lin(lin);
}

nap_lin operator +(id_shared_t a, nap_lin b) {
  num_var *var = new num_var(a);

  return nap_lin(new num_linear_term(1, var, b.lin));
}

analysis::api::num_var *var(id_shared_t id) {
  return new num_var(id);
}

analysis::api::num_var *var_temporary() {
  return var(rreil_builder::temporary());
}
