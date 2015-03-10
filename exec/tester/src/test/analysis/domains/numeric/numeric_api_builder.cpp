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

nab_lin::nab_lin(id_shared_t id) {
  builder = [=]() {
    num_var *var = new num_var(id);
    return new num_linear_term(var);
  };
}

nab_lin::nab_lin(vs_shared_t vs) {
  builder = [=]() {
    return new num_linear_vs(vs_shared_t(vs));
  };
}

nab_lin::nab_lin(int64_t val) {
  builder = [=]() {
    return new num_linear_vs(vs_shared_t(new vs_finite(val)));
  };
}

nab_lin operator +(id_shared_t a, nab_lin b) {
  lin_builder_t builder = [=] {
    num_var *var = new num_var(a);
    return new num_linear_term(1, var, b.builder());
  };
  return nab_lin(builder);
}

nab_lin operator +(nab_sf a, nab_lin b) {
  lin_builder_t builder = [=] {
    num_var *var = new num_var(a.id);
    return new num_linear_term(a.factor, var, b.builder());
  };
  return nab_lin(builder);
}

nab_sf operator *(int64_t factor, id_shared_t id) {
  return nab_sf { factor, id };
}

analysis::api::num_var *nab::var(id_shared_t id) {
  return gc(new num_var(id));
}

analysis::api::num_var *nab::var_temporary() {
  return var(rreil_builder::temporary());
}

analysis::api::num_expr *nab::expr(nab_lin lin) {
  return gc(new num_expr_lin(lin.builder()));
}

analysis::api::num_linear *nab::lin(nab_lin lin) {
  return gc(lin.builder());
}
