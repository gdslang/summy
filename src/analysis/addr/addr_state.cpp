/*
 * addr_state.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/addr/addr_state.h>
#include <iostream>
#include <assert.h>

using namespace std;
using namespace analysis::addr;

addr_state *analysis::addr::addr_state::domop(::analysis::domain_state *other) {
  addr_state *other_casted = dynamic_cast<addr_state*>(other);

  if(!this->address)
    return new addr_state(*other_casted);
  if(!other_casted->address)
    return new addr_state(*this);
  assert(address == other_casted->address);
  return new addr_state(*this);
}

addr_state *analysis::addr::addr_state::join(::analysis::domain_state *other, size_t current_node) {
  return domop(other);
}

addr_state *analysis::addr::addr_state::narrow(::analysis::domain_state *other, size_t current_node) {
  return domop(other);
}

addr_state *analysis::addr::addr_state::widen(::analysis::domain_state *other, size_t current_node) {
  return domop(other);
}

bool analysis::addr::addr_state::operator>=(const ::analysis::domain_state &other) const {
  addr_state const& other_casted = dynamic_cast<addr_state const&>(other);

  if(!other_casted.address)
    return true;
  else if(!address)
    return false;
  assert(address == other_casted.address);
  return true;
}

void analysis::addr::addr_state::put(std::ostream &out) const {
  if(address)
    cout << "Some(0x" << hex << address.value() << dec << ")";
  else
    cout << "None";
}
