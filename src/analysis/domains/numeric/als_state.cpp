/*
 * als_state.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/numeric/als_state.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/value_set/value_set.h>
#include <algorithm>

using namespace analysis;
using namespace analysis::api;
using namespace std;
using namespace summy;

void als_state::put(std::ostream &out) const {
  bool first = true;
  out << "{";
  for(auto &elem : elements) {
    id_shared_t id = elem.first;
    singleton_value_t const &aliases = elem.second;
    if(!first) out << ", ";
    else first = false;
    out << "P(" << *id << ") -> {";
    bool first = true;
    for(auto &alias : aliases) {
      if(first) first = false;
      else out << ", ";
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
  numeric_state *me_compat;
  numeric_state *other_compat;
  tie(ignore, me_compat, other_compat) = compat(this, &other_casted);
  bool result = *me_compat >= *other_compat;
  delete me_compat;
  delete other_compat;
  return result;
}

als_state *als_state::join(domain_state *other, size_t current_node) {
  als_state const *other_casted = dynamic_cast<als_state*>(other);
  numeric_state *me_compat;
  numeric_state *other_compat;
  elements_t elements_compat;
  tie(elements_compat, me_compat, other_compat) = compat(this, other_casted);
  als_state *result = new als_state(me_compat->join(other_compat, current_node), elements_compat);
  delete me_compat;
  delete other_compat;
  return result;
}

als_state *als_state::box(domain_state *other, size_t current_node) {
  als_state const *other_casted = dynamic_cast<als_state*>(other);
  return other_casted->copy();
}

void als_state::assign(api::num_var *lhs, api::num_expr *rhs) {
  set<num_var*> _vars = vars(rhs);
  id_set_t ids_new;
  for(auto &var : _vars) {
    auto var_it = elements.find(var->get_id());
    if(var_it != elements.end()) ids_new.insert(var_it->second.begin(), var_it->second.end());
  }
  if(ids_new.size() > 0)
    elements[lhs->get_id()] = ids_new;
  else
    elements.erase(lhs->get_id());
  child_state->assign(lhs, rhs);
}

void als_state::assume(api::num_expr_cmp *cmp) {
  child_state->assume(cmp);
}

void als_state::assume(api::num_var *lhs, ptr_set_t aliases) {
  if(is_bottom()) return;
  for(auto &alias : aliases) {
    elements[lhs->get_id()].insert(alias.id);
    num_expr *e = new num_expr_lin(new num_linear_vs(alias.offset));
    child_state->assign(lhs, e);
    delete e;
  }
}

void als_state::kill(std::vector<api::num_var*> vars) {
  for(auto &var : vars) {
//    cout << "ALS removing " << *var << endl;
    auto var_it = elements.find(var->get_id());
    if(var_it != elements.end()) elements.erase(var_it);
  }
  child_state->kill(vars);
}

void als_state::equate_kill(num_var_pairs_t vars) {
  for(auto var_pair : vars) {
    num_var *a, *b;
    tie(a, b) = var_pair;
    auto b_it = elements.find(b->get_id());
    if(b_it != elements.end() && b_it->second.size() > 0)
      elements[a->get_id()] = b_it->second;
    elements.erase(b->get_id());
  }
  child_state->equate_kill(vars);
}

void als_state::fold(num_var_pairs_t vars) {
  child_state->fold(vars);
}

api::ptr_set_t analysis::als_state::queryAls(api::num_var *nv) {
  ptr_set_t result;
  auto id_it = elements.find(nv->get_id());
  if(id_it == elements.end()) return child_state->queryAls(nv);
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
   * Todo: merge with eval() of vsd_state
   */
  num_visitor nv;
  vs_shared_t result;
  nv._([&](num_linear_term *lt) {
    vs_shared_t scale = vs_finite::single(lt->get_scale());
    vs_shared_t id = queryVal(lt->get_var());
    vs_shared_t next = queryVal(lt->get_next());
    result = *(*scale * id) + next;
  });
  nv._([&](num_linear_vs *lvs) {
    result = lvs->get_value_set();
  });
  lin->accept(nv);
  return result;
}

summy::vs_shared_t analysis::als_state::queryVal(api::num_var *nv) {
  vs_shared_t child_value = child_state->queryVal(nv);
  auto id_set_it = elements.find(nv->get_id());
  if(id_set_it == elements.end() || id_set_it->second.size() == 0)
    return child_value;
  vs_shared_t acc;
  bool first = true;
  for(auto id : id_set_it->second) {
    num_var *child_var = new num_var(id);
    vs_shared_t offset = child_state->queryVal(child_var);
    vs_shared_t next = *child_value + offset;
    if(first) {
      acc = next;
      first = false;
    } else
      acc = value_set::join(next, acc);
  }
  return acc;
}

als_state *als_state::copy() const {
  return new als_state(*this);
}

std::tuple<elements_t, numeric_state*, numeric_state*> analysis::als_state::compat(const als_state *a,
    const als_state *b) {
  numeric_state *a_ = a->child_state->copy();
  numeric_state *b_ = b->child_state->copy();
  elements_t r;
  auto single = [&](id_shared_t id, numeric_state *n) {
    num_var *nv = new num_var(id);
    num_expr *top_expr = new num_expr_lin(new num_linear_vs(value_set::top));
    n->assign(nv, top_expr);
    delete nv;
    delete top_expr;
  };
  for(auto &x : a->elements) {
    auto x_b_it = b->elements.find(x.first);
    if(x_b_it == b->elements.end()) single(x.first, a_);
    else {
      id_set_t combined;
      set_union(x.second.begin(), x.second.end(), x_b_it->second.begin(), x_b_it->second.end(),
          inserter(combined, combined.begin()));
      r.insert(make_pair(x.first, combined));
    }
  }
  for(auto &x : b->elements) {
    auto x_a_it = a->elements.find(x.first);
    if(x_a_it == a->elements.end()) single(x.first, b_);
  }
  return make_tuple(r, a_, b_);
}

