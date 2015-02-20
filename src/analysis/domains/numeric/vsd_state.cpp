/*
 * vsd_state.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/analysis/domains/api/num_visitor.h>
#include <summy/value_set/value_set.h>
#include <string>

using namespace summy;
using namespace analysis;
using namespace analysis::api;
using namespace std;

summy::vs_shared_t value_sets::vsd_state::lookup(id_shared_t id) {
  auto id_it = elements.find(id);
  if(id_it != elements.end())
    return id_it->second;
  else
    return value_set::top;
}

vs_shared_t value_sets::vsd_state::eval(num_linear *lin) {
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

numeric_state *value_sets::vsd_state::assign(num_var *lhs, num_expr *rhs) {
}
