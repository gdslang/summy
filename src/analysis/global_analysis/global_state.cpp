/*
 * global_state.cpp
 *
 *  Created on: Apr 23, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/global_analysis/global_state.h>
#include <algorithm>
#include <assert.h>

using namespace std;
using namespace analysis;
using namespace summy;

analysis::global_state::~global_state() {
  delete this->mstate;
}

global_state *analysis::global_state::join(::analysis::domain_state *other, size_t current_node) {
  global_state *other_casted = dynamic_cast<global_state *>(other);

  summary_memory_state *mstate_joined = mstate->join(other_casted->mstate, current_node);
  vs_shared_t f_addr_joined = value_set::join(this->f_addr, other_casted->f_addr);

  return new global_state(mstate_joined, f_addr_joined);
}

global_state *analysis::global_state::narrow(::analysis::domain_state *other, size_t current_node) {
//  cout << "NARROWING" << endl;

  global_state *other_casted = dynamic_cast<global_state *>(other);

  summary_memory_state *mstate_narrowed = mstate->narrow(other_casted->mstate, current_node);
  vs_shared_t f_addr_narrowed = value_set::narrow(this->f_addr, other_casted->f_addr);

  return new global_state(mstate_narrowed, f_addr_narrowed);
}

global_state *analysis::global_state::widen(::analysis::domain_state *other, size_t current_node) {
//  cout << "WIDENING" << endl;

  global_state *other_casted = dynamic_cast<global_state *>(other);

  summary_memory_state *mstate_widened = mstate->widen(other_casted->mstate, current_node);
//  cout << "widen(" << *this->f_addr << ", " << *other_casted->get_f_addr() <<")" << endl;
  vs_shared_t f_addr_widened = value_set::join(this->f_addr, other_casted->f_addr);

  return new global_state(mstate_widened, f_addr_widened);
}

bool analysis::global_state::operator >=(const ::analysis::domain_state &other) const {
  global_state const &other_casted = dynamic_cast<global_state const &>(other);
  bool f_addr_ge = *other_casted.f_addr <= this->f_addr;

  return f_addr_ge && (*other_casted.f_addr <= f_addr) && (*mstate >= *other_casted.mstate);
}

void analysis::global_state::check_consistency() {
  mstate->check_consistency();
}

void analysis::global_state::put(std::ostream &out) const {
  out << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
  out << *mstate << endl;
  out << "f_addr = " << *f_addr << endl;
  out << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%";
}
