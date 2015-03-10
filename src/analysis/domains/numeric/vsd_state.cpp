/*
 * vsd_state.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/analysis/domains/api/num_linear.h>
#include <summy/analysis/domains/api/num_expr.h>
#include <summy/analysis/domains/api/num_visitor.h>
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

void value_sets::vsd_state::put(std::ostream &out) {
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
  cout << "IN EVAL" << endl;
  nv._([&](num_linear_term *lt) {
    vs_shared_t scale = vs_finite::single(lt->get_scale());
    cout << "Scale: " << lt->get_scale() << endl;
    vs_shared_t id = lookup(lt->get_var()->get_id());
    cout << "Lookup result: " << *id << endl;
    vs_shared_t next = eval(lt->get_next());
    cout << *(id) << endl;
    cout << *(scale) << endl;
    cout << *(*scale * id) << endl;
//    cout << "... >>>" << endl;
//    cout << *scale << endl;
//    cout << *id << endl;
//    cout << *next << endl;
//    cout << "... <<<" << endl;
    result = *(*scale * id) + next;
  });
  nv._([&](num_linear_vs *lvs) {
    result = lvs->get_value_set();
    cout << "+++ " << *result << endl;
  });
  lin->accept(nv);
  cout << "LEAVE EVAL " << *result << endl;
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

bool analysis::value_sets::vsd_state::operator >=(domain_state &other) {
  return true;
}

vsd_state *analysis::value_sets::vsd_state::join(domain_state *other, size_t current_node) {
  return NULL;
}

vsd_state *analysis::value_sets::vsd_state::box(domain_state *other, size_t current_node) {
  return NULL;
}

vsd_state *value_sets::vsd_state::assign(num_var *lhs, num_expr *rhs) {
  elements_t elements_new = elements;
  elements_new[lhs->get_id()] = eval(rhs);
  return new vsd_state(elements_new);
}
