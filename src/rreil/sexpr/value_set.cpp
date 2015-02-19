/*
 * value_set.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */

#include <include/summy/rreil/sexpr/value_set_sexpr.h>
#include <summy/rreil/sexpr/sexpr_visitor.h>

using gdsl::rreil::sexpr_visitor;

void summy::rreil::value_set_sexpr::put(std::ostream &out) {
  out << *inner;
}

summy::rreil::value_set_sexpr::~value_set_sexpr() {
}

void summy::rreil::value_set_sexpr::accept(gdsl::rreil::sexpr_visitor &v) {
  summy::rreil::sexpr_visitor &vs = dynamic_cast<summy::rreil::sexpr_visitor&>(v);
  vs.visit(this);
}
