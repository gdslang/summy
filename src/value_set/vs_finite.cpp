/*
 * vs_finite.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */

#include <summy/value_set/vs_finite.h>
#include <bjutil/printer.h>

void summy::vs_finite::put(std::ostream &out) {
  out << print(elements);
}

void summy::vs_finite::accept(value_set_visitor &v) {
  v.visit(this);
}
