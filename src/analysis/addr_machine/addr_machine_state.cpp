/*
 * addr_state.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: Julian Kranz
 */

#include <iostream>
#include <assert.h>
#include <summy/analysis/addr_machine/addr_machine_state.h>

using namespace std;
using namespace analysis::addr_machine;

// addr_state

addr_machine_state *analysis::addr_machine::addr_machine_state::domop(::analysis::domain_state *other) {
  addr_machine_state *other_casted = dynamic_cast<addr_machine_state *>(other);

  if(!this->address) return new addr_machine_state(*other_casted);
  if(!other_casted->address) return new addr_machine_state(*this);

  if(this->address.value() != other_casted->address.value())
    return new addr_machine_state();

  return new addr_machine_state(*this);
}

addr_machine_state *analysis::addr_machine::addr_machine_state::join(::analysis::domain_state *other, size_t current_node) {
//    cout << *this << " LUB " << *other << endl;
  auto r = domop(other);
//    cout << "  = " << *r << endl;
  return r;
}

addr_machine_state *analysis::addr_machine::addr_machine_state::narrow(::analysis::domain_state *other, size_t current_node) {
  return new addr_machine_state(*this);
}

addr_machine_state *analysis::addr_machine::addr_machine_state::widen(::analysis::domain_state *other, size_t current_node) {
  return domop(other);
}

bool analysis::addr_machine::addr_machine_state::operator>=(const ::analysis::domain_state &other) const {
  addr_machine_state const &other_casted = dynamic_cast<addr_machine_state const &>(other);

  if(!other_casted.address)
    return true;
  else if(!address)
    return false;
  auto &addr_value = address.value();
  auto &addr_value_other = other_casted.address.value();
  return addr_value == addr_value_other;
}

void analysis::addr_machine::addr_machine_state::put(std::ostream &out) const {
  if(address)
    out << "Some(addr=0x" << hex << address.value() << dec << ")";
  else
    out << "None";
}
