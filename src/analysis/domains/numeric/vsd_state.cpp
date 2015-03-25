/*
 * vsd_state.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_finite.h>
#include <bjutil/printer.h>
#include <string>
#include <memory>

using namespace summy;
using namespace analysis;
using namespace analysis::api;
using namespace analysis::value_sets;

using namespace std;

void value_sets::vsd_state::put(std::ostream &out) const {
//  map<int, int*> a;
//  out << print(a, stream<int>(), stream_ptr<int*>());
//  out << print(elements, stream_ptr<singleton_key_t>(), stream_ptr<singleton_value_t>());
  if(is_bottom()) {
    cout << "⊥";
    return;
  }
  out << "{";
  bool first = true;
  for(auto &elem_it : elements) {
    if(first) first = false;
    else out << ", ";
    out << *elem_it.first << " -> " << *elem_it.second;
  }
  out << "}";
}

vs_shared_t value_sets::vsd_state::eval(num_linear *lin) {
  num_visitor nv;
  vs_shared_t result;
  nv._([&](num_linear_term *lt) {
    vs_shared_t scale = vs_finite::single(lt->get_scale());
    vs_shared_t id = lookup(lt->get_var()->get_id());
    vs_shared_t next = eval(lt->get_next());
    result = *(*scale * id) + next;
  });
  nv._([&](num_linear_vs *lvs) {
    result = lvs->get_value_set();
  });
  lin->accept(nv);
  return result;
}

vs_shared_t value_sets::vsd_state::eval(num_expr *exp) {
  num_visitor nv;
  vs_shared_t result;
  nv._([&](num_expr_cmp *cmp) {
    throw string("value_sets::vsd_state::eval");
  });
  nv._([&](num_expr_lin *lin) {
    result = eval(lin->get_inner());
  });
  nv._([&](num_expr_bin *bin) {
    vs_shared_t opnd1 = eval(bin->get_opnd1());
    vs_shared_t opnd2 = eval(bin->get_opnd2());
    switch(bin->get_op()) {
      case MUL: {
        result = *opnd1 * opnd2;
        break;
      }
      case DIV: {
        result = *opnd1 / opnd2;
        break;
      }
      default: {
        result = value_set::top;
        break;
      }
    }
  });
  exp->accept(nv);
  return result;
}

summy::vs_shared_t value_sets::vsd_state::lookup(id_shared_t id) {
  auto id_it = elements.find(id);
  if(id_it != elements.end()) return id_it->second;
  else return value_set::top;
}

bool analysis::value_sets::vsd_state::operator >=(domain_state const &other) const {
  vsd_state const &other_casted = dynamic_cast<vsd_state const&>(other);
  if(other_casted.is_bottom()) return true;
  else if(is_bottom()) return false;
  for(auto &mapping_mine : elements) {
    auto mapping_other = other_casted.elements.find(mapping_mine.first);
    if(mapping_other != other_casted.elements.end()) {
      if(!(*mapping_other->second <= mapping_mine.second)) return false;
    } else if(!(*mapping_mine.second == value_set::top)) return false;
  }
  return true;
}

vsd_state *analysis::value_sets::vsd_state::join(domain_state *other, size_t current_node) {
  vsd_state *other_casted = dynamic_cast<vsd_state*>(other);
  if(other_casted->is_bottom()) return new vsd_state(*this);
  else if(is_bottom()) return new vsd_state(*other_casted);

  elements_t elems_new;
  auto join = [&](elements_t const &from, elements_t const &to) {
    for(auto &mapping_first : from) {
      auto mapping_second = to.find(mapping_first.first);
      if(mapping_second != to.end())
      elems_new[mapping_first.first] = value_set::join(mapping_first.second, mapping_second->second);
//        cout << "join(" << *mapping_first.second << ", " << *mapping_second->second << ") = " << *elems_new[mapping_first.first] << endl;
    }
  };
  join(elements, other_casted->elements);
  join(other_casted->elements, elements);

  return new vsd_state(elems_new);
}

vsd_state *analysis::value_sets::vsd_state::widen(domain_state *other, size_t current_node) {
  vsd_state *other_casted = dynamic_cast<vsd_state*>(other);
  if(other_casted->is_bottom()) return new vsd_state(*this);
  elements_t elements_new;
  for(auto &mapping_other : other_casted->elements)
    elements_new[mapping_other.first] = value_set::widen(lookup(mapping_other.first), mapping_other.second);
  return new vsd_state(elements_new);
}

vsd_state *analysis::value_sets::vsd_state::narrow(domain_state *other, size_t current_node) {
  vsd_state *other_casted = dynamic_cast<vsd_state*>(other);
  if(other_casted->is_bottom()) return new vsd_state(*this);
  elements_t elements_new;
  for(auto &mapping_other : other_casted->elements)
    elements_new[mapping_other.first] = value_set::narrow(lookup(mapping_other.first), mapping_other.second);
  return new vsd_state(elements_new);
}

vsd_state *analysis::value_sets::vsd_state::box(domain_state *other, size_t current_node) {
  if(*other <= *this) return this->narrow(other, current_node);
  else return this->widen(other, current_node);
}

void value_sets::vsd_state::assign(num_var *lhs, num_expr *rhs) {
  vs_shared_t er = eval(rhs);
  _is_bottom = _is_bottom || *er == value_set::bottom;
  if(is_bottom())
    return;
  elements[lhs->get_id()] = er;
}

void analysis::value_sets::vsd_state::assume(api::num_expr_cmp *cmp) {
  throw string("analysis::value_sets::vsd_state::assume(num_expr_cmp)");
}

void analysis::value_sets::vsd_state::assume(api::num_var *lhs, ptr_set_t aliases) {
  if(is_bottom())
    return;
//  throw string("analysis::value_sets::vsd_state::assume(num_var, ptr_set_t)");
}

void analysis::value_sets::vsd_state::kill(std::vector<api::num_var*> vars) {
  for(auto var : vars) {
    auto var_it = elements.find(var->get_id());
    if(var_it != elements.end()) elements.erase(var_it);
  }
}

void analysis::value_sets::vsd_state::equate_kill(num_var_pairs_t vars) {
  for(auto var_pair : vars) {
    num_var *a, *b;
    tie(a, b) = var_pair;
    elements[a->get_id()] = elements[b->get_id()];
    elements.erase(b->get_id());
  }
}

void analysis::value_sets::vsd_state::fold(num_var_pairs_t vars) {
  throw string("analysis::value_sets::vsd_state::assume(num_var_pairs_t)");
}

vsd_state *analysis::value_sets::vsd_state::bottom() {
  return new vsd_state(true);
}

vsd_state* analysis::value_sets::vsd_state::top() {
  return new vsd_state();
}

api::ptr_set_t analysis::value_sets::vsd_state::queryAls(api::num_var *nv) {
  return ptr_set_t { };
}

summy::vs_shared_t analysis::value_sets::vsd_state::queryVal(num_linear *lin) {
//  vs_shared_t result;
//  num_visitor nv;
//  nv._([&](num_linear_term *n) {
//    vs_shared_t next = queryVal(n->get_next());
//    result = *(*vs_finite::single(n->get_scale()) * queryVal(n->get_var())) + next;
//  });
//  nv._([&](num_linear_vs *n) {
//    result = n->get_value_set();
//  });
//  lin->accept(nv);
//  return result;
  return eval(lin);
}

summy::vs_shared_t analysis::value_sets::vsd_state::queryVal(api::num_var *nv) {
//  auto id_it = elements.find(nv->get_id());
//  if(id_it == elements.end()) throw string(
//      "analysis::value_sets::vsd_state::queryVal(api::num_var*): unknown variable");
//  return id_it->second;
  return lookup(nv->get_id());
}

numeric_state *analysis::value_sets::vsd_state::copy() const {
  return new vsd_state(*this);
}
