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
class num_linear;
class num_expr_cmp;
}

typedef std::vector<std::tuple<api::num_var*, api::num_var*>> num_var_pairs_t;

class numeric_state : public domain_state {
public:
  virtual void bottomify() = 0;
  virtual bool is_bottom() const = 0;

  virtual void assign(api::num_var *lhs, api::num_expr *rhs) = 0;
  virtual void weak_assign(api::num_var *lhs, api::num_expr *rhs) = 0;
  virtual void assume(api::num_expr_cmp *cmp) = 0;
  virtual void assume(api::num_var *lhs, api::ptr_set_t aliases) = 0;
  virtual void kill(std::vector<api::num_var*> vars) = 0;
  virtual void equate_kill(num_var_pairs_t vars) = 0;
  virtual void fold(num_var_pairs_t vars) = 0;

  virtual bool cleanup(api::num_var *var) = 0;

  virtual numeric_state *join(domain_state *other, size_t current_node) = 0;
  virtual numeric_state *box(domain_state *other, size_t current_node) = 0;

  virtual api::ptr_set_t queryAls(api::num_var *nv) = 0;
  virtual summy::vs_shared_t queryVal(api::num_linear *lin) = 0;
  virtual summy::vs_shared_t queryVal(api::num_var *nv) = 0;

  virtual numeric_state *copy() const = 0;
};

}
