
/*
 * equality_state.cpp
 *
 *  Created on: Apr 2, 2015
 *      Author: Julian Kranz
 */
#include <summy/analysis/domains/numeric/equality_state.h>
#include <summy/analysis/domains/api/api.h>
#include <iostream>
#include <functional>
#include <algorithm>
#include <assert.h>

using namespace std;
using namespace analysis;
using namespace analysis::api;
using namespace gdsl::rreil;
using namespace summy;

api::num_linear *analysis::equality_state::simplify(api::num_linear *l) {
  map<id_shared_t, int64_t, id_less_no_version> id_scales;
  num_linear *lin_simplified;
  num_visitor nv;
  nv._([&](num_linear_term *nt) {
    id_shared_t id = nt->get_var()->get_id();
    auto id_back_it = back_map.find(id);
    if(id_back_it != back_map.end())
      id = id_back_it->second;
    auto id_scales_it = id_scales.find(id);
    if(id_scales_it == id_scales.end())
      id_scales.insert(make_pair(id, nt->get_scale()));
    else
      id_scales_it->second += nt->get_scale();
    nt->get_next()->accept(nv);
  });
  nv._([&](num_linear_vs *ns) {
    lin_simplified = ns->copy();
    for(auto id_scales_mapping : id_scales) {
      int64_t scale = id_scales_mapping.second;
      if(scale == 0)
        continue;
      lin_simplified = new num_linear_term(scale, new num_var(id_scales_mapping.first), lin_simplified);
    }
  });
  l->accept(nv);
  return lin_simplified;
}

api::num_expr *analysis::equality_state::simplify(api::num_expr *e) {
  num_expr *e_simplified;
  num_visitor nv;
  nv._([&](num_expr_cmp *nec) {
    e_simplified = new num_expr_cmp(simplify(nec->get_opnd()), nec->get_op());
  });
  nv._([&](num_expr_lin *nel) {
    e_simplified = new num_expr_lin(simplify(nel->get_inner()));
  });
  nv._([&](num_expr_bin *neb) {
    e_simplified = new num_expr_bin(simplify(neb->get_opnd1()), neb->get_op(), simplify(neb->get_opnd2()));
  });
  e->accept(nv);
  return e_simplified;
}

void analysis::equality_state::remove(api::num_var *v) {
  auto id_back_it = back_map.find(v->get_id());
  if(id_back_it != back_map.end()) {
    auto id_elements_it = elements.find(id_back_it->second);
    assert(id_elements_it != elements.end());
    id_set_t &id_elements = id_elements_it->second;
    id_elements.erase(v->get_id());
    if(id_elements.size() <= 1) {
      if(id_elements.size() == 1)
        back_map.erase(*id_elements.begin());
      elements.erase(id_elements_it);
    } else if(*v->get_id() == *id_back_it->second) {
      // new repr.
      id_shared_t rep = *id_elements.begin();
      for(auto &elem : id_elements)
        back_map[elem] = rep;
      elements[rep] = id_elements;
      elements.erase(id_elements_it);
    }
    back_map.erase(id_back_it);
  }
}

void analysis::equality_state::assign_var(api::num_var *lhs, api::num_var *rhs) {
//    cout << "assign in equality_state: " << *lhs << " <- " << *rhs << endl;

  auto insert = [&](id_shared_t id, id_shared_t rep) {
    //      cout << "Insert " << *id << " / rep: " << *rep << endl;
      auto rep_it = elements.find(rep);
      rep_it->second.insert(id);
      if(**rep_it->second.begin() == *id) {
        for(auto &elem : rep_it->second)
        back_map[elem] = id;
        elements[id] = rep_it->second;
        elements.erase(rep_it);
      } else
      back_map[id] = rep;
    };

  /*
   * Remove equality set of lhs
   */
  remove(lhs);

  /*
   * Lookup equlity set of rhs
   */
  auto rhs_back_it = back_map.find(rhs->get_id());
  if(rhs_back_it == back_map.end()) {
    tie(rhs_back_it, ignore) = back_map.insert(make_pair(rhs->get_id(), rhs->get_id()));
    elements[rhs_back_it->second].insert(rhs->get_id());
  }

  /*
   * Assign equlity set of lhs to lhs
   */
  back_map[lhs->get_id()] = rhs_back_it->second;

  /*
   * Insert lhs into the equality set of rhs
   */
  insert(lhs->get_id(), rhs_back_it->second);
}

void analysis::equality_state::weak_assign_var(api::num_var *lhs, api::num_var *rhs) {
//    cout << "assign in equality_state: " << *lhs << " <- " << *rhs << endl;
  auto lhs_back_it = back_map.find(lhs->get_id());
  if(lhs_back_it != back_map.end()) {
    auto rhs_back_it = back_map.find(rhs->get_id());
    if(rhs_back_it == back_map.end() || !(*lhs_back_it->second == *rhs_back_it->second))
      remove(lhs);
  }
}

void analysis::equality_state::assign_lin(api::num_var *lhs, api::num_linear *lin,
    void (equality_state::*assigner)(api::num_var*, api::num_var*)) {
  num_visitor nv;
  nv._([&](num_linear_term *nt) {
    if(nt->get_scale() == 0)
    assign_lin(lhs, nt->get_next(), assigner);
    else if(nt->get_scale() == 1) {
      vs_shared_t rest_vs = queryVal(nt->get_next());
      if(*rest_vs == vs_finite::single(0))
        (this->*assigner)(lhs, nt->get_var());
      else
        remove(lhs);
    } else
      remove(lhs);
  });
  nv._default([&]() {
    remove(lhs);
  });
  lin->accept(nv);
}

void analysis::equality_state::assign_expr(api::num_var *lhs, api::num_expr *rhs,
    void (equality_state::*assigner)(api::num_var*, api::num_var*)) {
  num_visitor nv;
  nv._([&](num_expr_lin *e) {
    assign_lin(lhs, e->get_inner(), assigner);
  });
  nv._default([&]() {
    remove(lhs);
  });
  rhs->accept(nv);
}

void analysis::equality_state::put(std::ostream &out) const {
  bool first = true;
  out << "{";
  for(auto &elem : elements) {
    id_shared_t id = elem.first;
    singleton_value_t const &aliases = elem.second;
    if(!first) out << ", ";
    else first = false;
    out << "Eq(" << *id << ") -> {";
    bool first = true;
    for(auto &alias : aliases) {
      if(first) first = false;
      else out << ", ";
      out << *alias;
    }
    out << "}";
  }
  out << " ### {";
  first = true;
  for(auto &elem : back_map) {
    id_shared_t id = elem.first;
    if(!first) out << ", ";
    else first = false;
    out << "back(" << *id << ") -> ";
    out << *elem.second;
  }
  out << "}" << endl;
  out << "Child state: {" << endl;
  out << *child_state;
  out << endl << "}";
}

analysis::equality_state::~equality_state() {
  delete child_state;
}

void analysis::equality_state::bottomify() {
  elements.clear();
  back_map.clear();
  child_state->bottomify();
}

bool analysis::equality_state::is_bottom() const {
  return child_state->is_bottom();
}

bool analysis::equality_state::operator >=(const domain_state &other) const {
  equality_state const &other_casted = dynamic_cast<equality_state const&>(other);
  if(*child_state >= *other_casted.child_state) {
    for(auto &back_mapping : back_map) {
      id_set_t const &ids = elements.at(back_mapping.second);
      if(ids.size() > 1) {
        auto other_back_mapping = other_casted.back_map.find(back_mapping.first);
        if(other_back_mapping == other_casted.back_map.end())
          return false;
        id_set_t const &ids_other = other_casted.elements.at(other_back_mapping->second);
        if(!includes(ids_other.begin(), ids_other.end(), ids.begin(), ids.end()))
          return false;
      }
    }
    return true;
  } else
    return false;
}

equality_state *analysis::equality_state::join(domain_state *other, size_t current_node) {
  equality_state const *other_casted = dynamic_cast<equality_state*>(other);
  back_map_t back_map_joined;
  eq_elements_t eq_elements_joined;
  for(auto &back_mapping : back_map) {
    id_set_t const &ids = elements.at(back_mapping.second);
    auto other_back_mapping = other_casted->back_map.find(back_mapping.first);
    if(other_back_mapping == other_casted->back_map.end())
      continue;
    id_set_t const &ids_other = other_casted->elements.at(other_back_mapping->second);

    id_set_t ids_joined;
    set_intersection(ids.begin(), ids.end(), ids_other.begin(), ids_other.end(), inserter(ids_joined, ids_joined.begin()));
    if(ids_joined.size() > 1) {
      id_shared_t rep = *ids_joined.begin();
      for(auto &id_joined : ids_joined)
        back_map_joined[id_joined] = rep;
      eq_elements_joined[rep] = ids_joined;
    }
  }

  return new equality_state(child_state->join(other_casted->child_state, current_node), eq_elements_joined, back_map_joined);
}

equality_state *analysis::equality_state::box(domain_state *other, size_t current_node) {
  equality_state const *other_casted = dynamic_cast<equality_state*>(other);
  return other_casted->copy();
}

void analysis::equality_state::assign(api::num_var *lhs, api::num_expr *rhs) {
//  cout << "assign expression in equality_state: " << *lhs << " <- " << *rhs << endl;
  num_expr *rhs_simplified = simplify(rhs);
  assign_expr(lhs, rhs_simplified, &equality_state::assign_var);
  child_state->assign(lhs, rhs_simplified);
  delete rhs_simplified;
}

void analysis::equality_state::weak_assign(api::num_var *lhs, api::num_expr *rhs) {
  num_expr *rhs_simplified = simplify(rhs);
  assign_expr(lhs, rhs_simplified, &equality_state::weak_assign_var);
  child_state->weak_assign(lhs, rhs_simplified);
  delete rhs_simplified;
}

void analysis::equality_state::assume(api::num_expr_cmp *cmp) {
  /*
   * Besser: Statt quadratisch alle Kombinationen zu testen,
   * sollte man besser zunächst die Reps begrenzen und alle
   * anderen Variablen dann lediglich noch gegen diese begrenzen...
   */
  struct gen {
    id_set_t ids;
    gen *rest;
    function<num_linear*(gen &_this)> _;

    id_set_t::iterator ids_it;

    gen(id_set_t ids, gen *rest, function<num_linear*(gen &_this)> _) :
      ids(ids), rest(rest), _(_) {
      ids_it = this->ids.begin();
    }
    ~gen() {
      delete rest;
    }

    bool end() {
      return ids_it == ids.end();
    }

    void next() {
      if(!rest)
        return;
//      rest->next();
      if(rest->end()) {
        if(ids_it == ids.end())
          ids_it = ids.begin();
        ids_it++;
        rest->next();
      }
    }

    num_linear *generate() {
      num_linear *r = _(*this);
      next();
      return r;
    }
  };

  function<gen*(num_linear *lin)> generate_gen;
  generate_gen = [&](num_linear *lin) {
    gen *g;
    num_visitor nv;
    nv._([&](num_linear_term *nt) {
      id_set_t ids;
      auto back_map_it = back_map.find(nt->get_var()->get_id());
      if(back_map_it == back_map.end())
        ids.insert(nt->get_var()->get_id());
      else
        ids = elements[back_map_it->second];
      g = new gen(
        ids,
        generate_gen(nt->get_next()),
        [=](gen &_this) {
          return new num_linear_term(nt->get_scale(), new num_var(*_this.ids_it), _this.rest->generate());
      });
    });
    nv._([&](num_linear_vs *ns) {
      g = new gen(
        id_set_t(),
        NULL,
        [=](gen &_this) {
          return new num_linear_vs(ns->get_value_set());
      });
    });
    lin->accept(nv);
    return g;
  };

  gen *g = generate_gen(cmp->get_opnd());

  while(!g->end()) {
    num_expr_cmp *ec = new num_expr_cmp(g->generate(), cmp->get_op());
    child_state->assume(ec);
    delete ec;
  }

  delete g;
}

void analysis::equality_state::assume(api::num_var *lhs, api::ptr_set_t aliases) {
  child_state->assume(lhs, aliases);
}

void analysis::equality_state::kill(std::vector<api::num_var*> vars) {
  for(num_var *v : vars)
    remove(v);
  child_state->kill(vars);
}

void analysis::equality_state::equate_kill(num_var_pairs_t vars) {
  for(auto &vp : vars) {
    num_var *lhs;
    num_var *rhs;
    tie(lhs, rhs) = vp;
    assign_var(lhs, rhs);
    remove(rhs);
  }
  child_state->equate_kill(vars);
}

void analysis::equality_state::fold(num_var_pairs_t vars) {
  throw string("analysis::equality_state::fold(num_var_pairs_t)");
}

bool analysis::equality_state::cleanup(api::num_var *var) {
  bool var_required = true;
  auto var_it = back_map.find(var->get_id());
  if(var_it != back_map.end())
    if(elements[var_it->second].size() < 2) {
      remove(var);
      var_required = false;
    }
  bool child_required = child_state->cleanup(var);
  return var_required || child_required;
}

api::ptr_set_t analysis::equality_state::queryAls(api::num_var *nv) {
  return child_state->queryAls(nv);
}

summy::vs_shared_t analysis::equality_state::queryVal(api::num_linear *lin) {
  num_linear *lin_simplified = simplify(lin);
  vs_shared_t result = child_state->queryVal(lin_simplified);
  delete lin_simplified;
  return result;
}

summy::vs_shared_t analysis::equality_state::queryVal(api::num_var *nv) {
  return child_state->queryVal(nv);
}

equality_state *analysis::equality_state::copy() const {
  return new equality_state(*this);
}
