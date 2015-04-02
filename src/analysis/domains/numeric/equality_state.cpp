
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

using namespace std;
using namespace analysis;
using namespace analysis::api;
using namespace gdsl::rreil;
using namespace summy;

void analysis::equality_state::put(std::ostream &out) const {
//  bool first = true;
//  out << "{";
//  for(auto &elem : elements) {
//    id_shared_t id = elem.first;
//    singleton_value_t const &aliases = elem.second;
//    if(!first) out << ", ";
//    else first = false;
//    out << "Eq(" << *id << ") -> {";
//    bool first = true;
//    for(auto &alias : aliases) {
//      if(first) first = false;
//      else out << ", ";
//      out << *alias;
//    }
//    out << "}";
//  }
//  out << "}" << endl;
//  out << "Child state: {" << endl;
//  out << *child_state;
//  out << endl << "}";
}

analysis::equality_state::equality_state(const equality_state &o) :
    child_state(o.child_state->copy()) {
  for(auto &back_mapping : o.back_map) {
    id_set_t *s = new id_set_t(*back_mapping.second);
    back_map.insert(make_pair(back_mapping.first, s));
    elements.insert(s);
  }
}

analysis::equality_state::~equality_state() {
  delete child_state;
}

bool analysis::equality_state::is_bottom() const {
  return child_state->is_bottom();
}

bool analysis::equality_state::operator >=(const domain_state &other) const {
  if(*child_state >= other) {
    /*
     * Todo: broken
     */
    return true;
  } else
    return false;
}

equality_state *analysis::equality_state::join(domain_state *other, size_t current_node) {
  equality_state const *other_casted = dynamic_cast<equality_state*>(other);
  return new equality_state(child_state->join(other_casted->child_state, current_node), elements_t {}, back_map_t {});
}

equality_state *analysis::equality_state::box(domain_state *other, size_t current_node) {
  equality_state const *other_casted = dynamic_cast<equality_state*>(other);
  return other_casted->copy();
}

void analysis::equality_state::assign(api::num_var *lhs, api::num_expr *rhs) {
  auto assign_var = [&](api::num_var *rhs_var) {
    auto lhs_back_it = back_map.find(lhs->get_id());
    if(lhs_back_it != back_map.end()) {
      id_set_t *eqs = lhs_back_it->second;
      eqs->erase(lhs->get_id());
      if(eqs->size() == 0) {
        elements.erase(eqs);
        delete eqs;
      }
    }
    auto rhs_back_it = back_map.find(rhs_var->get_id());
    if(rhs_back_it == back_map.end())
      tie(rhs_back_it, ignore) = back_map.insert(make_pair(rhs_var->get_id(), new id_set_t()));
    id_set_t *eqs_rhs = rhs_back_it->second;
    rhs_back_it->second->insert(lhs->get_id());
    if(lhs_back_it != back_map.end())
      lhs_back_it->second = eqs_rhs;
    else
      back_map.insert(make_pair(lhs->get_id(), eqs_rhs));
  };

  function<void(num_linear*)> assign_lin;
  assign_lin = [&](num_linear *lin) {
    num_visitor nv(true);
    nv._([&](num_linear_term *nt) {
      if(nt->get_scale() == 0)
        assign_lin(nt->get_next());
      else if(nt->get_scale() == 1) {
        vs_shared_t rest_vs = queryVal(nt->get_next());
        if(*rest_vs == vs_finite::single(0))
          assign_var(nt->get_var());
      }
    });
    lin->accept(nv);
  };

  num_visitor nv(true);
  nv._([&](num_expr_lin *e) {
    assign_lin(e->get_inner());
  });
  rhs->accept(nv);
}

void analysis::equality_state::weak_assign(api::num_var *lhs, api::num_expr *rhs) {
  /*
   * Todo: broken
   */
  child_state->weak_assign(lhs, rhs);
}

void analysis::equality_state::assume(api::num_expr_cmp *cmp) {
}

void analysis::equality_state::assume(api::num_var *lhs, api::ptr_set_t aliases) {
  /*
   * Todo: broken
   */
  child_state->assume(lhs, aliases);
}

void analysis::equality_state::kill(std::vector<api::num_var*> vars) {
  /*
   * Todo: broken
   */
  child_state->kill(vars);
}

void analysis::equality_state::equate_kill(num_var_pairs_t vars) {
  /*
   * Todo: broken
   */
  child_state->equate_kill(vars);
}

void analysis::equality_state::fold(num_var_pairs_t vars) {
  throw string("analysis::equality_state::fold(num_var_pairs_t)");
}

bool analysis::equality_state::cleanup(api::num_var *var) {
  /*
   * broken
   */
  return child_state->cleanup(var);
}

api::ptr_set_t analysis::equality_state::queryAls(api::num_var *nv) {
  return child_state->queryAls(nv);
}

summy::vs_shared_t analysis::equality_state::queryVal(api::num_linear *lin) {
  return child_state->queryVal(lin);
}

summy::vs_shared_t analysis::equality_state::queryVal(api::num_var *nv) {
  return child_state->queryVal(nv);
}

equality_state *analysis::equality_state::copy() const {
  return new equality_state(*this);
}

//std::tuple<elements_t, numeric_state*, numeric_state*> analysis::equality_state::compat(const als_state* a,
//    const als_state* b) {
//}
