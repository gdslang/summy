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
#include <map>
#include <optional>

namespace analysis {

enum link_type {
  DYNAMIC, STATIC
};

struct sm_function_desc {
  std::string name;
  link_type lt;
};

typedef std::map<size_t, sm_function_desc> fmap_t;

struct symbol {
  std::string symbol_name;
  void *address;
};

class static_memory {
public:
  virtual std::optional<std::reference_wrapper<fmap_t>> functions() = 0;
  virtual bool read(void *address, size_t bytes, uint8_t *buffer) const = 0;
  virtual std::tuple<bool, symbol> lookup(void *address) const = 0;
//  virtual bool check(void *address, size_t bytes) const = 0;
  virtual ~static_memory() {
  }
};

class static_dummy : public static_memory {
public:
  std::optional<std::reference_wrapper<fmap_t>> functions();
  bool read(void *address, size_t bytes, uint8_t *buffer) const;
  std::tuple<bool, symbol> lookup(void *address) const;
};

class static_elf : public static_memory {
private:
  elf_provider *ep;
  std::optional<fmap_t> fmap;
public:
  static_elf(elf_provider *ep) : ep(ep) {
  }

  std::optional<std::reference_wrapper<fmap_t>> functions();
  bool read(void *address, size_t bytes, uint8_t *buffer) const;
  std::tuple<bool, symbol> lookup(void *address) const;
};

}
