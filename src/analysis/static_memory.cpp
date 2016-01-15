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

// static_dummy

std::experimental::optional<reference_wrapper<fmap_t>> analysis::static_dummy::functions_all() {
  return std::experimental::nullopt;
}

bool static_dummy::read(void *address, size_t bytes, uint8_t *buffer) const {
  return false;
}


std::tuple<bool, symbol> analysis::static_dummy::lookup(void *address) const {
  return make_tuple(false, symbol());
}

// static_elf

std::experimental::optional<reference_wrapper<fmap_t>> analysis::static_elf::functions_all() {
  if(!this->fmap) {
    this->fmap = fmap_t();
    auto for_functions = [&](auto functions) {
      for(tuple<string, binary_provider::entry_t> fdesc : functions) {
        string name;
        binary_provider::entry_t entry;
        tie(name, entry) = fdesc;
        this->fmap.value()[entry.address] = name;
      }
    };
    for_functions(ep->functions());
    for_functions(ep->functions_dynamic());
  }
  return reference_wrapper<fmap_t>(this->fmap.value());
}

bool static_elf::read(void *address, size_t bytes, uint8_t *buffer) const {
  return ep->deref(address, bytes, buffer);
}

std::tuple<bool, symbol> analysis::static_elf::lookup(void *address) const {
  bool success;
  slice s;
  tie(success, s) = ep->deref_slice(address);
  symbol symb = {s.symbol, s.address};
  return make_tuple(success, symb);
}
