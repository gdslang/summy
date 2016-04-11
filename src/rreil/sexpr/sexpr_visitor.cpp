/*
 * sexpr_visitor.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */

#include <summy/rreil/sexpr/value_set_sexpr.h>
#include <summy/rreil/sexpr/sexpr_visitor.h>

void summy::rreil::sexpr_visitor::visit(value_set_sexpr *v) {
  if(value_set_callback != NULL)
    value_set_callback(v);
  else
    _default(v);
}
