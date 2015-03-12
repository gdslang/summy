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
  out << "{";
  bool first = true;
  for(auto &elem_it : elements) {
    if(first)
      first = false;
    else
      out << ", ";
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
  if(id_it != elements.end())
    return id_it->second;
  else
    return value_set::top;
}

bool analysis::value_sets::vsd_state::operator >=(domain_state const &other) const {
  vsd_state const &other_casted = dynamic_cast<vsd_state const&>(other);
  for(auto &mapping_mine : elements) {
    auto mapping_other = other_casted.elements.find(mapping_mine.first);
    if(mapping_other != other_casted.elements.end()) {
      if(!(*mapping_other->second <= mapping_mine.second))
        return false;
    } else
      if(!(*mapping_mine.second == value_set::top))
        return false;
  }
  return true;
}

vsd_state *analysis::value_sets::vsd_state::join(domain_state *other, size_t current_node) {
  vsd_state *other_casted = dynamic_cast<vsd_state*>(other);

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
  elements_t elements_new;
  vsd_state *other_casted = dynamic_cast<vsd_state*>(other);
  for(auto &mapping_other : other_casted->elements)
      elements_new[mapping_other.first] = value_set::widen(lookup(mapping_other.first), mapping_other.second);
  return new vsd_state(elements_new);
}

vsd_state *analysis::value_sets::vsd_state::narrow(domain_state *other, size_t current_node) {
  elements_t elements_new;
  vsd_state *other_casted = dynamic_cast<vsd_state*>(other);
  for(auto &mapping_other : other_casted->elements)
      elements_new[mapping_other.first] = value_set::narrow(lookup(mapping_other.first), mapping_other.second);
  return new vsd_state(elements_new);
}

vsd_state *analysis::value_sets::vsd_state::box(domain_state *other, size_t current_node) {
  if(*other <= *this)
    return this->narrow(other, current_node);
  else
    return this->widen(other, current_node);
}

vsd_state *value_sets::vsd_state::assign(num_var *lhs, num_expr *rhs) {
  elements_t elements_new = elements;
  elements_new[lhs->get_id()] = eval(rhs);
  return new vsd_state(elements_new);
}

numeric_state *analysis::value_sets::vsd_state::assume(api::num_expr_cmp *cmp) {
  throw string("analysis::value_sets::vsd_state::assume(num_expr_cmp)");
}

numeric_state *analysis::value_sets::vsd_state::assume(api::num_var *lhs, anaylsis::api::ptr_set_t aliases) {
  throw string("analysis::value_sets::vsd_state::assume(num_var, ptr_set_t)");
}

numeric_state *analysis::value_sets::vsd_state::kill(std::vector<api::num_var*> vars) {
  throw string("analysis::value_sets::vsd_state::assume(std::vector<api::num_var*>)");
}

numeric_state *analysis::value_sets::vsd_state::equate_kill(num_var_pairs_t vars) {
  throw string("analysis::value_sets::vsd_state::assume(num_var_pairs_t)");
}

numeric_state *analysis::value_sets::vsd_state::fold(num_var_pairs_t vars) {
  throw string("analysis::value_sets::vsd_state::assume(num_var_pairs_t)");
}
