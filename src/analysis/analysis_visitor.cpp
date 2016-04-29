/*
 * analysis_visitor.cpp
 *
 *  Created on: Apr 28, 2016
 *      Author: Julian Kranz
 */
#include <summy/analysis/analysis_visitor.h>
#include <assert.h>

void analysis::analysis_visitor::visit(summary_dstack *v) {
  if(summary_dstack_callback != NULL) summary_dstack_callback(v);
  else _default();
}

void analysis::analysis_visitor::visit(fp_analysis *v) {
  _default();
}

void analysis::analysis_visitor::_default() {
  if(default_callback != NULL) default_callback();
  else assert(ignore_default);
}
