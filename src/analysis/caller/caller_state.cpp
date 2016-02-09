/*
 * caller_state.cpp
 *
 *  Created on: Feb 09, 2016
 *      Author: Julian Kranz
 */

#include <iostream>
#include <assert.h>
#include <summy/analysis/caller/caller_state.h>
#include <algorithm>
#include <iterator>
#include <bjutil/printer.h>

using namespace std;
using namespace analysis::caller;

caller_state *analysis::caller::caller_state::join(::analysis::domain_state *other, size_t current_node) {
  caller_state *other_casted = dynamic_cast<caller_state *>(other);

  set<size_t> callers_new;
  set_union(this->callers.begin(), this->callers.end(), other_casted->callers.begin(), other_casted->callers.end(),
    inserter(callers_new, callers_new.begin()));

  return new caller_state(callers_new);
}

caller_state *analysis::caller::caller_state::narrow(::analysis::domain_state *other, size_t current_node) {
  return new caller_state(*this);
}

caller_state *analysis::caller::caller_state::widen(::analysis::domain_state *other, size_t current_node) {
  return join(other, current_node);
}

caller_state *analysis::caller::caller_state::add_caller(size_t caller) {
  set<size_t> callers = this->callers;
  callers.insert(caller);

  return new caller_state(callers);
}

bool analysis::caller::caller_state::operator>=(const ::analysis::domain_state &other) const {
  caller_state const &other_casted = dynamic_cast<caller_state const &>(other);

  return includes(this->callers.begin(), this->callers.end(), other_casted.callers.begin(), other_casted.callers.end());
}

void analysis::caller::caller_state::put(std::ostream &out) const {
  out << print(callers) << endl;
}
