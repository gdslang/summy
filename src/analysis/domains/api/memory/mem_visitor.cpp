/*
 * mem_visitor.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/memory/mem_visitor.h>
#include <string>

using namespace analysis::api;
using namespace std;

void mem_visitor::visit(mem_expr_re *v) {
  if(mem_expr_re_callback != NULL) mem_expr_re_callback(v);
  else _default();
}

void mem_visitor::visit(mem_expr_deref *v) {
  if(mem_expr_deref_callback != NULL) mem_expr_deref_callback(v);
  else _default();
}

void mem_visitor::_default() {
  if(default_callback != NULL) default_callback();
  else if(!ignore_default)
    throw string("Unhandled default");
}
