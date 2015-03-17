/*
 * numeric.h
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domain_state.h>
#include <summy/analysis/domains/api/ptr_set.h>
#include <cppgdsl/rreil/id/id.h>
#include <memory>
#include <vector>
#include <tuple>

namespace analysis {
namespace api {
class num_var;
class num_expr;
class num_expr_cmp;
}

typedef std::vector<std::tuple<api::num_var*, api::num_var*>> num_var_pairs_t;

class numeric_state : public domain_state {
public:
  virtual bool is_bottom() = 0;

  virtual numeric_state *assign(api::num_var *lhs, api::num_expr *rhs) = 0;
  virtual numeric_state *assume(api::num_expr_cmp *cmp) = 0;
  virtual numeric_state *assume(api::num_var *lhs, anaylsis::api::ptr_set_t aliases) = 0;
  virtual numeric_state *kill(std::vector<api::num_var*> vars) = 0;
  virtual numeric_state *equate_kill(num_var_pairs_t vars) = 0;
  virtual numeric_state *fold(num_var_pairs_t vars) = 0;
};

}
