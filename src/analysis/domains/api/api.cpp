/*
 * api.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/api.h>

using namespace std;
using namespace analysis::api;

static void  _vars(set<num_var*> &current, num_linear *lin) {
  num_visitor nv;
  nv._([&](num_linear_term *n) {
    current.insert(n->get_var());
    _vars(current, n->get_next());
  });
  nv._([&](num_linear_vs *n) {
  });
  lin->accept(nv);
}

std::set<num_var*> analysis::api::vars(num_expr *expr) {
  set<num_var*> result;
  num_visitor nv;
  nv._([&](num_expr_cmp *n) {

  });
  nv._([&](num_expr_lin *n) {
    _vars(result, n->get_inner());
  });
  nv._([&](num_expr_bin *n) {
  });
  expr->accept(nv);
  return result;
}
