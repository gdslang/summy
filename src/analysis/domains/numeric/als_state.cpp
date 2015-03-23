/*
 * als_state.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/numeric/als_state.h>
#include <summy/analysis/domains/api/api.h>

using namespace analysis;
using namespace analysis::api;
using namespace std;

void als_state::put(std::ostream &out) const {
//  bool first = true;
//  out << "{";
//  for(auto &elem : elements) {
//    id_shared_t id = elem.first;
//    ptr_set_t const &aliases = elem.second;
//    if(!first)
//      out << ' ';
//    else
//      first = false;
//    out << "P(" << *id << ") -> ";
//    for(auto &alias : aliases)
//      out << alias;
//  }
//  out << "}" << endl;
  out << "Child state: {" << endl;
  out << *child_state;
  out << endl << "}";
}

analysis::als_state::~als_state() {
  delete child_state;
}

bool als_state::is_bottom() const {
  return child_state->is_bottom();
}

bool als_state::operator >=(const domain_state &other) const {
  als_state const &other_casted = dynamic_cast<als_state const&>(other);
  return *child_state >= *other_casted.child_state;
}

als_state *als_state::join(domain_state *other, size_t current_node) {
  als_state const *other_casted = dynamic_cast<als_state*>(other);
  /*
   * Broken, broken...
   */
  return new als_state(child_state->join(other_casted->child_state, 42), elements);
}

als_state *als_state::box(domain_state *other, size_t current_node) {
  als_state const *other_casted = dynamic_cast<als_state*>(other);
  /*
   * Broken, broken...
   */
  return new als_state(child_state->box(other_casted->child_state, 42), elements);
}

void als_state::assign(api::num_var *lhs, api::num_expr *rhs) {
  child_state->assign(lhs, rhs);
}

void als_state::assume(api::num_expr_cmp *cmp) {
  child_state->assume(cmp);
}

void als_state::assume(api::num_var *lhs, ptr_set_t aliases) {
  elements[lhs->get_id()].insert(aliases.begin(), aliases.end());
  child_state->assume(lhs, aliases);
}

void als_state::kill(std::vector<api::num_var*> vars) {
  child_state->kill(vars);
}

void als_state::equate_kill(num_var_pairs_t vars) {
  child_state->equate_kill(vars);
}

void als_state::fold(num_var_pairs_t vars) {
  child_state->fold(vars);
}

numeric_state *als_state::copy() {
  return new als_state(*this);
}
