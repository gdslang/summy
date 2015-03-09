/*
 * numeric_api_builder.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: Julian Kranz
 */
#include <summy/test/analysis/domains/numeric_api_builder.h>
//#include <summy/value_set/vs_finite.h>

using namespace summy;
using namespace analysis::api;

nap_lin::nap_lin(id_shared_t id) {
  num_var *var = new num_var(id);
  lin = new num_linear_term(var);
}

nap_lin::nap_lin(summy::vs_shared_t vs) {
  lin = new num_linear_vs(vs);
}

nap_lin operator +(id_shared_t a, nap_lin b) {
  num_var *var = new num_var(a);

  return nap_lin(new num_linear_term(1, var, b.lin));
}
