/*
 * domain_state.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domain_state.h>

using namespace analysis;

bool domain_state::operator <=(domain_state &other) {
  return other >= *this;
}
