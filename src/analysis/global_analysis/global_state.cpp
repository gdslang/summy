/*
 * global_state.cpp
 *
 *  Created on: Apr 23, 2015
 *      Author: Julian Kranz
 */


#include <summy/analysis/global_analysis/global_state.h>

using namespace std;
using namespace analysis;


global_state *analysis::global_state::join(::analysis::domain_state *other, size_t current_node) {
  global_state *other_casted = dynamic_cast<global_state *>(other);

  summary_memory_state *returned_joined = returned->join(other_casted->returned.get(), current_node);
  summary_memory_state *consecutive_joined = consecutive->join(other_casted->consecutive.get(), current_node);

  return new global_state(shared_ptr<summary_memory_state>(returned_joined), shared_ptr<summary_memory_state>(consecutive_joined));
}

global_state *analysis::global_state::narrow(::analysis::domain_state *other, size_t current_node) {
  global_state *other_casted = dynamic_cast<global_state *>(other);

  summary_memory_state *returned_narrowed = returned->join(other_casted->returned.get(), current_node);
  summary_memory_state *consecutive_narrowed = consecutive->join(other_casted->consecutive.get(), current_node);

  return new global_state(shared_ptr<summary_memory_state>(returned_narrowed), shared_ptr<summary_memory_state>(consecutive_narrowed));
}

global_state *analysis::global_state::widen(::analysis::domain_state *other, size_t current_node) {
  global_state *other_casted = dynamic_cast<global_state *>(other);

  summary_memory_state *returned_widened = returned->narrow(other_casted->returned.get(), current_node);
  summary_memory_state *consecutive_widened = consecutive->narrow(other_casted->consecutive.get(), current_node);

  return new global_state(shared_ptr<summary_memory_state>(returned_widened), shared_ptr<summary_memory_state>(consecutive_widened));
}

global_state *analysis::global_state::apply_summary() {
  /*
   * Todo: Apply summary (returned) to consecutive and
   * return result
   */
  throw string("analysis::global_state::apply_summary()");
}


bool analysis::global_state::operator >=(const ::analysis::domain_state &other) const {
  global_state const &other_casted = dynamic_cast<global_state const &>(other);

  return (*returned >= *other_casted.returned) && (*consecutive >= *other_casted.consecutive);
}

void analysis::global_state::put(std::ostream &out) const {
  out << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
  out << "returned:" << endl;
  out << *returned << endl;
  out << "<<<<<<<<<<<<<<>>>>>>>>>>>>>>" << endl;
  out << "consecutive:" << endl;
  out << *consecutive << endl;
}
