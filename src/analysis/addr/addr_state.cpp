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

// path_virts

analysis::addr::path_virts_s::path_virts_s(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
  data[0] = a;
  data[1] = b;
  data[2] = c;
  data[3] = d;
}

analysis::addr::path_virts_s::path_virts_s(const path_virts_s &path_virts) {
  for(size_t i = 0; i < path_virts_s::n; i++)
    data[i] = path_virts[i];
}

uint64_t &analysis::addr::path_virts_s::operator[](size_t index) {
  return data[index];
}

uint64_t const &analysis::addr::path_virts_s::operator[](size_t index) const {
  return data[index];
}

// node_addr

bool analysis::addr::node_addr::operator<(const node_addr &other) const {
  if(machine < other.machine)
    return true;
  else if(machine > other.machine)
    return false;
  else
    return virt < other.virt;
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
  addr_state *other_casted = dynamic_cast<addr_state *>(other);

  if(!this->address) return new addr_state(*other_casted);
  if(!other_casted->address) return new addr_state(*this);

  bool me_subset = true;
  for(size_t i = 0; i < path_virts_s::n; i++) {
    if(!i)
      cout << "0x" << hex << path_virts[i] << " <==> " << other_casted->path_virts[i] << endl;
    me_subset = me_subset && ((path_virts[i] & other_casted->path_virts[i]) == path_virts[i]);
  }
  if(me_subset) return new addr_state(*this);

  bool other_subset = true;
  for(size_t i = 0; i < path_virts_s::n; i++)
    other_subset = other_subset && ((other_casted->path_virts[i] & path_virts[i]) == other_casted->path_virts[i]);

  if(other_subset) return new addr_state(*other_casted);

  auto &addr_value = address.value();
  auto &addr_value_other = other_casted->address.value();
  assert(addr_value.machine == addr_value_other.machine);
//  size_t virt = get_next_virt(addr_value.machine);
  size_t virt = max(addr_value.virt, addr_value_other.virt);
  cout << "MACHINE: " << addr_value.machine << endl;
  assert(virt < path_virts_s::n * path_virts_s::size_singleton_bits);
  path_virts_s path_virts;
  for(size_t i = 0; i < path_virts_s::n; i++) {
    path_virts[i] = this->path_virts[i] | other_casted->path_virts[i];
//    if(i == virt / path_virts_s::size_singleton_bits)
//      path_virts[i] |= (uint64_t)1 << (virt % path_virts_s::size_singleton_bits);
  }
  return new addr_state(node_addr(addr_value.machine, virt), path_virts, get_next_virt);
}

addr_state *analysis::addr::addr_state::join(::analysis::domain_state *other, size_t current_node) {
  cout << *this << " LUP " << *other << endl;
  auto r = domop(other);
  cout << "  = " << *r << endl;
  return r;
}

addr_state *analysis::addr::addr_state::narrow(::analysis::domain_state *other, size_t current_node) {
  return new addr_state(*this);
}

addr_state *analysis::addr::addr_state::widen(::analysis::domain_state *other, size_t current_node) {
  return domop(other);
}

addr_state *analysis::addr::addr_state::next_virt() {
  size_t machine = this->address.value().machine;
  size_t next_virt = get_next_virt(machine);
  cout << "MACHINE: " << machine << endl;
  assert(next_virt < path_virts_s::n * path_virts_s::size_singleton_bits);

  path_virts_s path_virts = this->path_virts;
  for(size_t i = 0; i < path_virts_s::n; i++)
    if(i == next_virt / path_virts_s::size_singleton_bits)
      path_virts[i] |= (uint64_t)1 << (next_virt % path_virts_s::size_singleton_bits);

  return new addr_state(node_addr(machine, next_virt), path_virts, get_next_virt);
}

bool analysis::addr::addr_state::operator>=(const ::analysis::domain_state &other) const {
  addr_state const &other_casted = dynamic_cast<addr_state const &>(other);

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
    out << "Some(0x" << hex << address.value() << ", 0x" << path_virts[0] << dec << ")";
  else
    out << "None";
}
