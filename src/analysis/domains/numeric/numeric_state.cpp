/*
 * numeric_state.cpp
 *
 *  Created on: Apr 9, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/numeric/numeric_state.h>
#include <iostream>

using namespace analysis;
using namespace std;

numeric_state *analysis::numeric_state::box(domain_state *other, size_t current_node) {
  if(*other <= *this) {
    cout << "Narrowing..." << endl;
    return this->narrow(other, current_node);
  }
  else {
    cout << "Widening..." << endl;
    return this->widen(other, current_node);
  }
}
