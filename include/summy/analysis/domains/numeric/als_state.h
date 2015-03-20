/*
 * als_state.h
 *
 *  Created on: Mar 20, 2015
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

typedef std::tuple<id_shared_t, api::ptr_set_t> singleton_t;
typedef std::tuple_element<0, singleton_t>::type singleton_key_t;
typedef std::tuple_element<1, singleton_t>::type singleton_value_t;
typedef std::map<singleton_key_t, singleton_value_t, id_less_no_version> elements_t;

/**
 * Alias domain state
 */
class als_state: public numeric_state {
private:
  numeric_state *child_state;
  elements_t elements;
protected:
  void put(std::ostream &out) const;
public:
  als_state(numeric_state *child_state, elements_t elements) :
    child_state(child_state), elements(elements) {
  }
  als_state(numeric_state *child_state) :
    child_state(child_state), elements(elements_t {}) {
  }
  als_state(als_state const&o) :
    child_state(o.child_state->copy()), elements(o.elements) {
  }
  ~als_state();

  const elements_t &get_elements() const {
    return elements;
  }

  bool is_bottom() const;

  bool operator>=(domain_state const &other) const;

  als_state *join(domain_state *other, size_t current_node);;
  als_state *box(domain_state *other, size_t current_node);

  void assign(api::num_var *lhs, api::num_expr *rhs);
  void assume(api::num_expr_cmp *cmp);
  void assume(api::num_var *lhs, api::ptr_set_t aliases);
  void kill(std::vector<api::num_var*> vars);
  void equate_kill(num_var_pairs_t vars);
  void fold(num_var_pairs_t vars);

  numeric_state *copy();
};

}
