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

nab_sf::operator nab_lin() {
  lin_builder_t builder = [=] {
    num_var *var = new num_var(id);
    return new num_linear_term(factor, var);
  };
  return nab_lin(builder);
}

analysis::api::num_var *nab::var(id_shared_t id) {
  return gc(new num_var(id));
}

analysis::api::num_expr *nab::expr(nab_lin lin) {
  return gc(new num_expr_lin(lin.builder()));
}

analysis::api::num_linear *nab::lin(nab_lin lin) {
  return gc(lin.builder());
}

nab_lin operator +(id_shared_t a, nab_lin b) {
  lin_builder_t builder = [=] {
    num_var *var = new num_var(a);
    return new num_linear_term(1, var, b.builder());
  };
  return nab_lin(builder);
}

nab_lin operator +(id_shared_t a, nab_sf b) {
  return a + (nab_lin)b;
}

nab_lin operator +(nab_sf a, nab_lin b) {
  lin_builder_t builder = [=] {
    num_var *var = new num_var(a.id);
    return new num_linear_term(a.factor, var, b.builder());
  };
  return nab_lin(builder);
}

nab_lin operator +(nab_lin a, nab_sf b) {
  return b + a;
}

nab_sf operator -(id_shared_t b) {
  return (-1) * b;
}

nab_sf operator *(int64_t factor, id_shared_t id) {
  return nab_sf { factor, id };
}
