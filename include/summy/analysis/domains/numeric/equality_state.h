/*
 * equality_state.h
 *
 *  Created on: Apr 2, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/numeric/numeric_state.h>
#include <summy/analysis/domains/ptr_set.h>
#include <summy/value_set/value_set.h>
#include <summy/analysis/util.h>
#include <cppgdsl/rreil/id/id.h>
#include <map>
#include <set>

namespace analysis {
namespace api {
class num_linear;
}

typedef std::map<id_shared_t, int64_t, id_less> id_off_map_t;
typedef std::tuple<id_shared_t, id_off_map_t> eq_singleton_t;
typedef std::tuple_element<0, eq_singleton_t>::type eq_singleton_key_t;
typedef std::tuple_element<1, eq_singleton_t>::type eq_singleton_value_t;
typedef std::map<eq_singleton_key_t, eq_singleton_value_t, id_less> eq_elements_t;
typedef std::map<id_shared_t, id_shared_t, id_less> back_map_t;

/**
 * Equality domain state
 */
class equality_state: public numeric_state {
private:
  numeric_state *child_state;
  eq_elements_t elements;
  back_map_t back_map;

  typedef numeric_state*(numeric_state::*domopper_t) (domain_state *other, size_t current_node);
  equality_state *join_widen(domain_state *other, size_t current_node, domopper_t domopper);

  api::num_linear *simplify(api::num_linear *l);
  api::num_expr *simplify(api::num_expr *e);

//  id_set_t const& lookup(api::num_var *v);
  void remove(api::num_var *v);
  void merge(api::num_var *v, api::num_var *w);
  void assign_var(api::num_var *lhs, api::num_var *rhs, int64_t offset);
  void weak_assign_var(api::num_var *lhs, api::num_var *rhs, int64_t offset);
  void assign_lin(api::num_var *lhs, api::num_linear *lin,
      void (equality_state::*assigner)(api::num_var*, api::num_var*, int64_t));
  void assign_expr(api::num_var *lhs, api::num_expr *rhs,
      void (equality_state::*assigner)(api::num_var*, api::num_var*, int64_t));
protected:
  void put(std::ostream &out) const;
public:
  equality_state(numeric_state *child_state, eq_elements_t elements, back_map_t back_map) :
      child_state(child_state), elements(elements), back_map(back_map) {
  }
  equality_state(numeric_state *child_state) :
      child_state(child_state), elements(eq_elements_t { }), back_map(back_map_t()) {
  }
  equality_state(equality_state const&o) :
      child_state(o.child_state->copy()), elements(o.elements), back_map(o.back_map) {
  }
  ~equality_state();

  const eq_elements_t &get_elements() const {
    return elements;
  }

  void bottomify();
  bool is_bottom() const;

  bool operator>=(domain_state const &other) const;

  equality_state *join(domain_state *other, size_t current_node);
  equality_state *meet(domain_state *other, size_t current_node);
  equality_state *widen(domain_state *other, size_t current_node);
  equality_state *narrow(domain_state *other, size_t current_node);

  void assign(api::num_var *lhs, api::num_expr *rhs);
  void assign(api::num_var *lhs, ptr_set_t aliases);
  void weak_assign(api::num_var *lhs, api::num_expr *rhs);
  void assume(api::num_expr_cmp *cmp);
  void assume(api::num_var *lhs, ptr_set_t aliases);
  void kill(std::vector<api::num_var*> vars);
  void equate_kill(num_var_pairs_t vars);
  void fold(num_var_pairs_t vars);
  void copy_paste(api::num_var *to, api::num_var *from, numeric_state *from_state);

  bool cleanup(api::num_var *var);
  void project(api::num_vars *vars);
  api::num_vars *vars();
  void collect_ids(std::map<gdsl::rreil::id const*, std::set<analysis::id_shared_t*>> &id_map);

  ptr_set_t queryAls(api::num_var *nv);
  summy::vs_shared_t queryVal(api::num_linear *lin);
  summy::vs_shared_t queryVal(api::num_var *nv);

  equality_state *copy() const;
};

}
