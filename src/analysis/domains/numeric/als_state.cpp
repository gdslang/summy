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
  bool als_a_ge_b;
  numeric_state *me_compat;
  numeric_state *other_compat;
  tie(als_a_ge_b, ignore, me_compat, other_compat) = compat(this, &other_casted);
  bool child_ge = *me_compat >= *other_compat;
  delete me_compat;
  delete other_compat;
  return als_a_ge_b && child_ge;
}

als_state *als_state::join(domain_state *other, size_t current_node) {
  als_state const *other_casted = dynamic_cast<als_state*>(other);
  numeric_state *me_compat;
  numeric_state *other_compat;
  elements_t elements_compat;
  tie(ignore, elements_compat, me_compat, other_compat) = compat(this, other_casted);
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
  bool linear = false;
  num_visitor nv(true);
  nv._([&](num_expr_lin *le) {
    linear = true;
  });
  rhs->accept(nv);
  if(linear) {
    set<num_var*> _vars = vars(rhs);
    id_set_t ids_new;
    for(auto &var : _vars) {
      auto var_it = elements.find(var->get_id());
      if(var_it != elements.end()) ids_new.insert(var_it->second.begin(), var_it->second.end());
    }
    if(ids_new.size() > 0) elements[lhs->get_id()] = ids_new;
    else elements.erase(lhs->get_id());
  } else
    elements.erase(lhs->get_id());
  child_state->assign(lhs, rhs);
}

void als_state::weak_assign(api::num_var *lhs, api::num_expr *rhs) {
  bool linear = false;
  num_visitor nv(true);
  nv._([&](num_expr_lin *le) {
    linear = true;
  });
  rhs->accept(nv);

  if(linear) {
    set<num_var*> _vars = vars(rhs);
    id_set_t ids_erase;

    auto ids_mine_it = elements.find(lhs->get_id());
    if(ids_mine_it != elements.end()) {
      id_set_t &ids_mine = ids_mine_it->second;
      id_set_t rest;

      for(auto &var : _vars) {
        auto var_it = elements.find(var->get_id());
        id_set_t const &ids_var = var_it->second;
        if(var_it != elements.end())
          set_intersection(ids_mine.begin(), ids_mine.end(), ids_var.begin(), ids_var.end(), inserter(rest, rest.begin()));
      }
      if(rest.size() > 0)
        ids_mine = rest;
      else
        elements.erase(ids_mine_it);
    }
  } else
    elements.erase(lhs->get_id());
  child_state->weak_assign(lhs, rhs);
}

void als_state::assume(api::num_expr_cmp *cmp) {
  /*
   * Todo: Allow pointer comparisons; current implementation looses
   * aliasing information...
   *
   * Todo: Update paper: Ignoring assumtions in case they contain pointers
   * is not a viable option since now variables are pointers 'by default'
   */
  auto _vars = vars(cmp);
  for(auto &var : _vars) {
    auto var_it = elements.find(var->get_id());
    if(var_it != elements.end()) {
      num_expr *current_val_expr = new num_expr_lin(new num_linear_vs(queryVal(var)));
//      cout << "Assigning " << *current_val_expr << " to " << *var << endl;
      child_state->assign(var, current_val_expr);
      delete current_val_expr;
      elements.erase(var_it);
    }
  }
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
//  cout << "queryALS for " << *nv << endl;
//  cout << "offset: " << *child_state->queryVal(nv) << endl;

  ptr_set_t result;
  auto id_it = elements.find(nv->get_id());
  if(id_it == elements.end()) return child_state->queryAls(nv);
  singleton_value_t &aliases = id_it->second;
  for(auto alias : aliases) {
//    num_var *nv = new num_var(alias);
    result.insert(ptr(alias, child_state->queryVal(nv)));
//    delete nv;
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
    delete child_var;
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

bool analysis::als_state::cleanup(api::num_var *var) {
  bool child_clean = child_state->cleanup(var);
  if(elements.find(var->get_id()) != elements.end())
    return true;
  /*
   * Todo: reverse map
   */
  for(auto &id_set_it : elements)
    if(id_set_it.second.find(var->get_id()) != id_set_it.second.end())
      return true;
  return child_clean;
}

std::tuple<bool, elements_t, numeric_state*, numeric_state*> analysis::als_state::compat(const als_state *a,
    const als_state *b) {
  bool als_a_ge_b = true;
  numeric_state *a_ = a->child_state->copy();
  numeric_state *b_ = b->child_state->copy();
  elements_t r;
  auto single = [&](id_shared_t id, numeric_state *n) {
    num_var *nv = new num_var(id);
    num_expr *top_expr = new num_expr_lin(new num_linear_vs(value_set::top));
    /*
     * Todo: more precision
     */
    n->assign(nv, top_expr);
    delete nv;
    delete top_expr;
  };
  for(auto &x : a->elements) {
    if(x.second.size() == 0)
      continue;
    auto x_b_it = b->elements.find(x.first);
    if(x_b_it == b->elements.end() || x_b_it->second.size() == 0) {
      single(x.first, a_);
      als_a_ge_b = false;
    } else {
//      cout << "Join of" << endl;
//      for(auto &u : x.second)
//        cout << *u << " ";
//      cout << endl << "and" << endl;
//      for(auto &u : x_b_it->second)
//        cout << *u << " ";
//      cout << endl << "is" << endl;
      id_set_t joined;
      set_union(x.second.begin(), x.second.end(), x_b_it->second.begin(), x_b_it->second.end(),
          inserter(joined, joined.begin()));
//      for(auto &u : joined)
//        cout << *u << " ";
//      cout << endl;
      r.insert(make_pair(x.first, joined));
      if(joined.size() > x.second.size())
        als_a_ge_b = false;
    }
  }
  for(auto &x : b->elements) {
    if(x.second.size() == 0)
      continue;
    auto x_a_it = a->elements.find(x.first);
    if(x_a_it == a->elements.end() || x_a_it->second.size() == 0) single(x.first, b_);
  }
  return make_tuple(als_a_ge_b, r, a_, b_);
}

