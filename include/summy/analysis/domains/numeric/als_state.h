/*
 * als_state.h
 *
 *  Created on: Mar 20, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <cppgdsl/rreil/id/id.h>
#include <map>
#include <set>
#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/domains/numeric/num_evaluator.h>
#include <summy/analysis/domains/numeric/numeric_state.h>
#include <summy/analysis/domains/ptr_set.h>
#include <summy/analysis/inverse_set_map.h>
#include <summy/analysis/util.h>
#include <summy/value_set/value_set.h>

namespace analysis {
namespace api {
class num_linear;
}

typedef std::set<id_shared_t, id_less> id_set_t;
typedef std::tuple<id_shared_t, id_set_t> singleton_t;
typedef std::tuple_element<0, singleton_t>::type singleton_key_t;
typedef std::tuple_element<1, singleton_t>::type singleton_value_t;
// typedef std::map<singleton_key_t, singleton_value_t, id_less_no_version> elements_t;
typedef inverse_set_map<id_shared_t, id_less> elements_t;

/**
 * Alias domain state
 */
class als_state : public numeric_state {
private:
  numeric_state *child_state;

  /*
   * - If a variable is not contained in elements, this is equivalent to an alias
   *   set containing the bad pointer and an unconstrained non-bad pointer only, i.e. top
   * - If an alias set of a variable is empty, the state of this variable is bottom
   */
  elements_t elements;

  void kill(id_shared_t id);
  ptr simplify_ptr_sum(std::vector<id_shared_t> const &pointers);
  api::num_expr *replace_pointers(api::num_expr *e);
  api::num_linear *replace_pointers(api::num_linear *l);
  api::num_linear *replace_pointers(
    std::map<id_shared_t, id_shared_t, id_less> &id_gen_map, api::num_linear *l);
  /*
   * Todo: Remove (siehe als_state.cpp oben)
   */
  typedef numeric_state *(numeric_state::*domopper_t)(domain_state *other, size_t current_node);
  als_state *domop(bool widening, domain_state *other, size_t current_node, domopper_t domopper);

  bool is_top(elements_t::const_iterator it) const;

protected:
  void put(std::ostream &out) const override;

public:
  als_state(numeric_state *child_state, elements_t elements);
  als_state(numeric_state *child_state) : als_state(child_state, elements_t{}) {}
  als_state(als_state const &o) : als_state(o.child_state->copy(), o.elements) {}
  ~als_state();

  const elements_t &get_elements() const {
    return elements;
  }

  void bottomify();
  bool is_bottom() const;

  bool operator>=(domain_state const &other) const;

  als_state *join(domain_state *other, size_t current_node);
  als_state *meet(domain_state *other, size_t current_node);
  als_state *widen(domain_state *other, size_t current_node);
  als_state *narrow(domain_state *other, size_t current_node);

  void assign(api::num_var *lhs, api::num_expr *rhs, bool weak);
  void assign(api::num_var *lhs, ptr_set_t aliases);
  void assign(api::num_var *lhs, api::num_expr *rhs);
  void weak_assign(api::num_var *lhs, api::num_expr *rhs);
  void assume(api::num_expr_cmp *cmp);
  void assume(api::num_var *lhs, ptr_set_t aliases);
  void kill(std::vector<api::num_var *> vars);
  void equate_kill(num_var_pairs_t vars);
  void fold(num_var_pairs_t vars);
  void copy_paste(api::num_var *to, api::num_var *from, numeric_state *from_state);

  bool cleanup(api::num_var *var);
  void project(api::num_vars *vars);
  api::num_vars *vars();
  void collect_ids(std::map<gdsl::rreil::id const *, std::set<analysis::id_shared_t *>> &id_map);

  ptr_set_t queryAls(api::num_var *nv);
  summy::vs_shared_t queryVal(api::num_linear *lin);
  summy::vs_shared_t queryVal(api::num_var *nv);

  als_state *copy() const;

  /**
   * Re-offset static memory aliases to the null pointer. Static memory ids
   * may in general be a bad idea; maybe, they should be removed.
   */
  static ptr_set_t normalise(ptr_set_t aliases);
  static std::tuple<bool, elements_t, numeric_state *, numeric_state *> compat(
    bool widening, als_state const *a, als_state const *b);
};
}
