/*
 * vs_top.cpp
 *
 *  Created on: Feb 12, 2015
 *      Author: Julian Kranz
 */

#include <summy/value_set/vs_top.h>
#include <summy/value_set/vs_open.h>
#include <summy/value_set/vs_finite.h>

using namespace summy;

void summy::vs_top::put(std::ostream &out) {
  out << "⊥";
}

vs_shared_t summy::vs_top::join(const vs_finite* vsf) {
}

vs_shared_t summy::vs_top::join(const vs_open* vsf) {
}

void summy::vs_top::accept(value_set_visitor &v) {
  v.visit(this);
}
