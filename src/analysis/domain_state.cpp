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
//  cout << "BOX of" << endl << *this << endl;
//  cout << "and" << endl;
//  cout << *other << endl;
  if(*other <= *this) {
//    cout << "NARROWING" << endl;
//    cout << "Narrowing no more :-(" << endl;
    return make_tuple(this->narrow(other, current_node), false);
  } else {
//    cout << "WIDENING" << endl;
    return make_tuple(this->widen(other, current_node), true);
  }
}

bool domain_state::operator <=(domain_state const &other) const {
  return other >= *this;
}

bool domain_state::operator ==(domain_state const &other) const {
  return other >= *this && *this >= other;
}
