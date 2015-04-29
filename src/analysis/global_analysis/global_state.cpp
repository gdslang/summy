/*
 * global_state.cpp
 *
 *  Created on: Apr 23, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/global_analysis/global_state.h>
#include <algorithm>

using namespace std;
using namespace analysis;

global_state *analysis::global_state::join(::analysis::domain_state *other, size_t current_node) {
  global_state *other_casted = dynamic_cast<global_state *>(other);

  summary_memory_state *mstate_joined = mstate->join(other_casted->mstate, current_node);
  callers_t callers_joined;
  set_union(callers.begin(), callers.end(), other_casted->callers.begin(), other_casted->callers.end(),
      inserter(callers_joined, callers_joined.begin()));

  /*
   * Todo: ??
   */
  size_t fstart_id;
  if(this->fstart_id != other_casted->fstart_id)
    fstart_id = current_node;
  else
    fstart_id = this->fstart_id;

  return new global_state(mstate_joined, fstart_id, callers_joined);
}

global_state *analysis::global_state::narrow(::analysis::domain_state *other, size_t current_node) {
  global_state *other_casted = dynamic_cast<global_state *>(other);
  return new global_state(*other_casted);
}

global_state *analysis::global_state::widen(::analysis::domain_state *other, size_t current_node) {
  global_state *other_casted = dynamic_cast<global_state *>(other);
  return new global_state(*other_casted);
}

bool analysis::global_state::operator >=(const ::analysis::domain_state &other) const {
  global_state const &other_casted = dynamic_cast<global_state const &>(other);
  bool callers_include = includes(callers.begin(), callers.end(), other_casted.callers.begin(), other_casted.callers.end());

  return callers_include && (fstart_id == other_casted.fstart_id) && (*mstate >= *other_casted.mstate);
}

void analysis::global_state::put(std::ostream &out) const {
  out << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
  out << *mstate << endl;
  out << "fstart_id = " << fstart_id << endl;
  out << "Callers: {";
  bool first = true;
  for(auto caller : callers) {
    if(first)
      first = false;
    else
      out << ", ";
    out << caller;
  }
  out << "}" << endl;
  out << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%";
}
