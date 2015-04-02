/*
 * equality_state.h
 *
 *  Created on: Apr 2, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/numeric/numeric_state.h>
#include <summy/analysis/domains/api/ptr_set.h>
#include <summy/value_set/value_set.h>
#include <summy/analysis/util.h>
#include <cppgdsl/rreil/id/id.h>
#include <map>
#include <set>

namespace analysis {
namespace api {
class num_linear;
}

typedef std::set<id_shared_t, id_less_no_version> id_set_t;
typedef std::set<id_set_t*> eq_elements_t;
typedef std::map<id_shared_t, id_set_t*, id_less_no_version> back_map_t;

/**
 * Equality domain state
 */
class equality_state: public numeric_state {
private:
  numeric_state *child_state;
  eq_elements_t elements;
  back_map_t back_map;
protected:
  void put(std::ostream &out) const;
public:
  equality_state(numeric_state *child_state, eq_elements_t elements, back_map_t back_map) :
      child_state(child_state), elements(elements), back_map(back_map) {
  }
  equality_state(numeric_state *child_state) :
      child_state(child_state), elements(eq_elements_t { }), back_map(back_map_t()) {
  }
  equality_state(equality_state const&o);
  ~equality_state();

  const eq_elements_t &get_elements() const {
    return elements;
  }

  bool is_bottom() const;

  bool operator>=(domain_state const &other) const;

  equality_state *join(domain_state *other, size_t current_node);
  equality_state *box(domain_state *other, size_t current_node);

  void assign(api::num_var *lhs, api::num_expr *rhs);
  void weak_assign(api::num_var *lhs, api::num_expr *rhs);
  void assume(api::num_expr_cmp *cmp);
  void assume(api::num_var *lhs, api::ptr_set_t aliases);
  void kill(std::vector<api::num_var*> vars);
  void equate_kill(num_var_pairs_t vars);
  void fold(num_var_pairs_t vars);

  bool cleanup(api::num_var *var);

  api::ptr_set_t queryAls(api::num_var *nv);
  summy::vs_shared_t queryVal(api::num_linear *lin);
  summy::vs_shared_t queryVal(api::num_var *nv);

  equality_state *copy() const;

//  static std::tuple<elements_t, numeric_state*, numeric_state*> compat(als_state const *a, als_state const *b);
};

}
