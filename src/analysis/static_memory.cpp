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

bool static_dummy::check(void *address, size_t bytes) const {
  return false;
}

bool static_elf::read(void *address, size_t bytes, uint8_t *buffer) const {
  return ep->deref(address, bytes, buffer);
}

bool static_elf::check(void *address, size_t bytes) const {
  return ep->check(address, bytes);
}
