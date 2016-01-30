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

// node_addr

bool analysis::addr::node_addr::operator<(const node_addr &other) const {
  if(machine < other.machine)
    return true;
  else if(machine > other.machine)
    return false;
  else return virt < other.virt;
}

bool analysis::addr::node_addr::operator<=(const node_addr &other) const {
  return *this < other || *this == other;
}

bool analysis::addr::node_addr::operator==(const node_addr &other) const {
  return machine == other.machine && virt == other.virt;
}

std::ostream &analysis::addr::operator<<(std::ostream &out, const node_addr &_this) {
  out << _this.machine << ":" << _this.virt;
  return out;
}

// addr_state

addr_state *analysis::addr::addr_state::domop(::analysis::domain_state *other) {
  addr_state *other_casted = dynamic_cast<addr_state*>(other);

  if(!this->address)
    return new addr_state(*other_casted);
  if(!other_casted->address)
    return new addr_state(*this);
  auto &addr_value = address.value();
  auto &addr_value_other = other_casted->address.value();
  assert(addr_value.machine == addr_value_other.machine);
  size_t virt = max(addr_value.virt, addr_value_other.virt);
  return new addr_state(node_addr(addr_value.machine, virt + 1));
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
  auto &addr_value = address.value();
  auto &addr_value_other = other_casted.address.value();
  return addr_value_other <= addr_value;
}

void analysis::addr::addr_state::put(std::ostream &out) const {
  if(address)
    out << "Some(0x" << hex << address.value() << dec << ")";
  else
    out << "None";
}
