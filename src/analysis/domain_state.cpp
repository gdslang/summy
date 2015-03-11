/*
 * domain_state.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domain_state.h>

using namespace analysis;

bool domain_state::operator <=(domain_state const &other) const {
  return other >= *this;
}

bool domain_state::operator ==(domain_state const &other) const {
  return other >= *this && *this >= other;
}
