/*
 * value_set_visitor.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */
#include <summy/value_set/value_set_visitor.h>
#include <summy/value_set/vs_finite.h>
#include <summy/value_set/vs_open.h>
#include <summy/value_set/vs_top.h>
#include <string>
#include <assert.h>

using namespace summy;
using namespace std;

void value_set_visitor::visit(vs_finite *v) {
  if(vs_finite_callback != NULL) vs_finite_callback(v);
  else _default(v);
}

void value_set_visitor::visit(vs_open *v) {
  if(vs_open_callback != NULL) vs_open_callback(v);
  else _default(v);
}

void value_set_visitor::visit(vs_top *v) {
  if(vs_top_callback != NULL) vs_top_callback(v);
  else _default(v);
}

void value_set_visitor::_default(value_set *v) {
  if(default_callback != NULL) default_callback(v);
  else assert(ignore_default);
}
