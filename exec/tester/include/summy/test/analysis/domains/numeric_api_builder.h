/*
 * numeric_api_builder.h
 *
 *  Created on: Mar 9, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <stdint.h>
#include <summy/test/rreil/rreil_builder.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/value_set/value_set.h>

struct nap_lin {
  analysis::api::num_linear *lin;

  nap_lin(id_shared_t id);
  nap_lin(summy::vs_shared_t vs);
  nap_lin(int64_t val);
  nap_lin(analysis::api::num_linear *lin) : lin(lin) {
  }

  analysis::api::num_expr *expr();
};

nap_lin operator +(id_shared_t a, nap_lin b);

analysis::api::num_var *var(id_shared_t id);
