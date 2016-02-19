/*
 * addr_state.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: Julian Kranz
 */

#include <iostream>
#include <assert.h>
#include <summy/analysis/addr/addr_state.h>

using namespace std;
using namespace analysis::addr;

// path_virts

analysis::addr::path_virts_s::path_virts_s(uint64_t a) {
  assert(path_virts_s::n > 0);
  data[0] = a;
  for(size_t i = 1; i < path_virts_s::n; i++)
    data[i] = 0;
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

bool analysis::addr::path_virts_s::operator<=(path_virts_s const& other) const {
  bool subset = true;
  for(size_t i = 0; i < path_virts_s::n; i++) {
    //    if(!i) cout << "0x" << hex << path_virts[i] << " <==> " << other_casted->path_virts[i] << endl;
    subset = subset && ((this->data[i] & other[i]) == this->data[i]);
  }
  return subset;
}

path_virts_s analysis::addr::path_virts_s::insert(size_t virt) const {
  assert(virt < path_virts_s::n * path_virts_s::size_singleton_bits);
  path_virts_s path_virts = *this;
  for(size_t i = 0; i < path_virts_s::n; i++)
    if(i == virt / path_virts_s::size_singleton_bits)
      path_virts[i] |= (uint64_t)1 << (virt % path_virts_s::size_singleton_bits);
  return path_virts;
}

path_virts_s analysis::addr::path_virts_s::_union(path_virts_s const& virts_other) const {
  path_virts_s path_virts;
  for(size_t i = 0; i < path_virts_s::n; i++)
    path_virts[i] = this->data[i] | virts_other[i];
  return path_virts;
}

std::ostream &analysis::addr::operator<<(std::ostream &out, const path_virts_s &_this) {
  out << "0x";
  for(size_t i = path_virts_s::n; i > 0; --i)
    if(_this[i - 1] != 0) out << _this[i - 1];
  return out;
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

  if(this->path_virts <= other_casted->path_virts)
    return new addr_state(*this);
  else if(other_casted->path_virts <= this->path_virts)
    return new addr_state(*other_casted);

  auto &addr_value = address.value();
  auto &addr_value_other = other_casted->address.value();
  assert(addr_value.machine == addr_value_other.machine);
  size_t virt = max(addr_value.virt, addr_value_other.virt);
  path_virts_s virts_unioned = this->path_virts._union(other_casted->path_virts);
  return new addr_state(node_addr(addr_value.machine, virt), virts_unioned, get_next_virt);
}

addr_state *analysis::addr::addr_state::join(::analysis::domain_state *other, size_t current_node) {
//    cout << *this << " LUB " << *other << endl;
  auto r = domop(other);
//    cout << "  = " << *r << endl;
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
  if(!this->next_virt_value)
    this->next_virt_value = get_next_virt(machine);
  path_virts_s path_virts = this->path_virts.insert(next_virt_value.value());
  return new addr_state(node_addr(machine, next_virt_value.value()), path_virts, get_next_virt);
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
    out << "Some(addr=0x" << hex << address.value() << ", virts=" << path_virts << dec << ")";
  else
    out << "None";
}
