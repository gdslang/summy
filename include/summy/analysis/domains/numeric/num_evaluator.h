/*
 * num_evaluator.h
 *
 *  Created on: Apr 11, 2015
 *      Author: jucs
 */

#pragma once

#include <summy/value_set/value_set.h>
#include <summy/analysis/domains/api/api.h>
#include <functional>

namespace analysis {

typedef std::function<summy::vs_shared_t(api::num_var*)> query_var_t;

class num_evaluator {
private:
  query_var_t query_var;

public:
  num_evaluator(query_var_t query_var) : query_var(query_var) {
  }

  summy::vs_shared_t queryVal(api::num_linear *lin);
  summy::vs_shared_t queryVal(api::num_expr *exp);
};

}
