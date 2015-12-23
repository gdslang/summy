/*
 * domain_state.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domain_state.h>

using namespace analysis;
using namespace std;

std::tuple<domain_state*, bool> domain_state::box(domain_state *other, size_t current_node) {
  cout << "BOX of" << endl << *other << endl;
  cout << "and" << endl;
  cout << *this << endl;
  if(*other <= *this) {
    cout << "NARROWING" << endl;
    return make_tuple(this->narrow(other, current_node), false);
  } else
    return make_tuple(this->widen(other, current_node), true);
}

bool domain_state::operator <=(domain_state const &other) const {
  return other >= *this;
}

bool domain_state::operator ==(domain_state const &other) const {
  return other >= *this && *this >= other;
}
