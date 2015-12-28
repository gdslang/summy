/*
 * num_visitor.cpp
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/numeric/num_visitor.h>
#include <summy/analysis/domains/api/numeric/num_expr.h>
#include <summy/analysis/domains/api/numeric/num_linear.h>
#include <string>
#include <assert.h>

using namespace analysis::api;
using namespace std;

void num_visitor::visit(num_linear_term *v) {
  if(num_linear_term_callback != NULL) num_linear_term_callback(v);
  else _default();
}

void num_visitor::visit(num_linear_vs *v) {
  if(num_linear_vs_callback != NULL) num_linear_vs_callback(v);
  else _default();
}

void num_visitor::visit(num_expr_cmp *v) {
  if(num_expr_cmp_callback != NULL) num_expr_cmp_callback(v);
  else _default();
}

void num_visitor::visit(num_expr_lin *v) {
  if(num_expr_lin_callback != NULL) num_expr_lin_callback(v);
  else _default();
}

void num_visitor::visit(num_expr_bin *v) {
  if(num_expr_bin_callback != NULL) num_expr_bin_callback(v);
  else _default();
}

void num_visitor::_default() {
  if(default_callback != NULL) default_callback();
  else assert(ignore_default);
}
