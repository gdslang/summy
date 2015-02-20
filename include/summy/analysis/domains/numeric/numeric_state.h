/*
 * numeric.h
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domain_state.h>
#include <summy/analysis/domains/api/num_var.h>
#include <summy/analysis/domains/api/num_expr.h>

namespace analysis {

class numeric_state : public domain_state {
public:
  virtual numeric_state *assign(api::num_var lhs, api::num_expr rhs) = 0;
};

}
