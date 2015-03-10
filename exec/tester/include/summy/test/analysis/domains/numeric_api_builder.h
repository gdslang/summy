/*
 * numeric_api_builder.h
 *
 *  Created on: Mar 9, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <stdint.h>
#include <functional>
#include <bjutil/autogc.h>
#include <summy/test/rreil/rreil_builder.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/value_set/value_set.h>

typedef std::function<analysis::api::num_linear*()> lin_builder_t;
struct nab_lin {
  lin_builder_t builder;

  nab_lin(id_shared_t id);
  nab_lin(summy::vs_shared_t vs);
  nab_lin(int64_t val);
  nab_lin(lin_builder_t builder) : builder(builder) {
  }
};
struct nab_sf {
  int64_t factor;
  id_shared_t id;

  operator nab_lin();

  nab_sf(id_shared_t id) : factor(1), id(id) {
  }
};

class nab {
private:
  autogc &gc;
public:
  nab(autogc &gc) : gc(gc) {
  }

  analysis::api::num_var *var(id_shared_t id);
  analysis::api::num_var *var_temporary();
  analysis::api::num_expr *expr(nab_lin lin);
  analysis::api::num_linear *lin(nab_lin lin);
};

//nab_lin operator +(id_shared_t a, nab_lin b);
nab_lin operator +(nab_sf a, nab_lin b);
nab_lin operator +(nab_lin a, nab_sf b);
nab_sf operator -(id_shared_t b);
nab_sf operator *(int64_t factor, id_shared_t id);

