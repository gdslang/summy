/*
 * static_memory.h
 *
 *  Created on: Apr 15, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <bjutil/binary/elf_provider.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <tuple>

namespace analysis {

struct symbol {
  std::string symbol_name;
  void *address;
};

class static_memory {
public:
  virtual bool read(void *address, size_t bytes, uint8_t *buffer) const = 0;
  virtual std::tuple<bool, symbol> lookup(void *address) const = 0;
//  virtual bool check(void *address, size_t bytes) const = 0;
  virtual ~static_memory() {
  }
};

class static_dummy : public static_memory {
public:
  bool read(void *address, size_t bytes, uint8_t *buffer) const;
  std::tuple<bool, symbol> lookup(void *address) const;
};

class static_elf : public static_memory {
private:
  elf_provider *ep;
public:
  static_elf(elf_provider *ep) : ep(ep) {
  }
  bool read(void *address, size_t bytes, uint8_t *buffer) const;
  std::tuple<bool, symbol> lookup(void *address) const;
};

}
