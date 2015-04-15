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

numeric_state::numeric_state() {
  this->sm = make_shared<static_dummy>();
}
