/*
 * static_memory.cpp
 *
 *  Created on: Apr 15, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/static_memory.h>
#include <tuple>
#include <iostream>

using namespace std;
using namespace analysis;

bool static_dummy::read(void *address, size_t bytes, uint8_t *buffer) const {
  return false;
}

std::tuple<bool, symbol> analysis::static_dummy::lookup(void* address) const {
  return make_tuple(false, symbol());
}

bool static_elf::read(void *address, size_t bytes, uint8_t *buffer) const {
  return ep->deref(address, bytes, buffer);
}

std::tuple<bool, symbol> analysis::static_elf::lookup(void *address) const {
  bool success;
  slice s;
  tie(success, s) = ep->deref_slice(address);
  symbol symb = { s.symbol, s.address };
  return make_tuple(success, symb);
}
