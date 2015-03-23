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


analysis::als_state::temp_s::~temp_s() {
  _this.child_state->kill({ temp });
  delete temp;
}

void als_state::put(std::ostream &out) const {
  bool first = true;
  out << "{";
  for(auto &elem : elements) {
    id_shared_t id = elem.first;
    singleton_value_t const &aliases = elem.second;
    if(!first)
      out << ", ";
    else
      first = false;
    out << "P(" << *id << ") -> {";
    bool first = true;
    for(auto &alias : aliases) {
      if(first)
        first = false;
      else
        out << ", ";
      out << *alias;
    }
    out << "}";
  }
  out << "}" << endl;
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
  cout << "ALS assign" << endl;
  set<num_var*> _vars = vars(rhs);
  for(auto &var : _vars) {
    cout << "lhs: " << *var->get_id() << endl;
    cout <<  "rhs var: " << *var->get_id() << endl;
    elements[lhs->get_id()] = elements[var->get_id()];
  }
  child_state->assign(lhs, rhs);
}

void als_state::assume(api::num_expr_cmp *cmp) {
  child_state->assume(cmp);
}

void als_state::assume(api::num_var *lhs, ptr_set_t aliases) {
  for(auto &alias : aliases) {
    elements[lhs->get_id()].insert(alias.id);
    num_expr *e = new num_expr_lin(new num_linear_vs(alias.offset));
    child_state->assign(lhs, e);
    delete e;
  }
}

void als_state::kill(std::vector<api::num_var*> vars) {
  for(auto &var : vars) {
    cout << "ALS removing " << *var << endl;

    auto var_it = elements.find(var->get_id());
    if(var_it != elements.end()) elements.erase(var_it);
  }
  child_state->kill(vars);
}

void als_state::equate_kill(num_var_pairs_t vars) {
  child_state->equate_kill(vars);
}

void als_state::fold(num_var_pairs_t vars) {
  child_state->fold(vars);
}

api::ptr_set_t analysis::als_state::queryAls(api::num_var *nv) {
  ptr_set_t result;
  auto id_it = elements.find(nv->get_id());
  if(id_it == elements.end())
    return result;
  singleton_value_t &aliases = id_it->second;
  for(auto alias : aliases) {
    num_var *nv = new num_var(alias);
    result.insert(ptr(alias, queryVal(nv)));
    delete nv;
  }
  return result;
}

summy::vs_shared_t analysis::als_state::queryVal(api::num_linear *lin) {
  /*
   * Todo: broken
   */
  return child_state->queryVal(lin);
}

summy::vs_shared_t analysis::als_state::queryVal(api::num_var *nv) {
  /*
   * Todo: broken
   */
  return child_state->queryVal(nv);
}

numeric_state *als_state::copy() {
  return new als_state(*this);
}
