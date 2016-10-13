/*
 * sm_id.h
 *
 *  Created on: Apr 16, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id.h>
#include <cppgdsl/rreil/id/id_visitor.h>
#include <summy/analysis/static_memory.h>
#include <iostream>
#include <assert.h>
#include <memory>
#include <string>

using analysis::symbol;

namespace summy {
namespace rreil {

/**
 * This class is used to represent pointers to static memory. Maybe
 * they should be removed and replaced by null pointer + offset.
 */
class sm_id : public gdsl::rreil::id {
private:
  std::string symbol;
  void *address;

  void put(std::ostream &out) const override;

  static size_t subclass_counter;
public:
  sm_id(std::string symbol, void *address) : symbol(symbol), address(address) {}
  ~sm_id();

  size_t get_subclass_counter() const override {
    return subclass_counter;
  }

  std::string get_symbol() const {
    return symbol;
  }

  void *get_address() const {
    return address;
  }

  bool operator==(gdsl::rreil::id const &other) const override;
  bool operator<(id const& other) const override;
  std::unique_ptr<gdsl::rreil::id> copy() const override {
    assert(false);
  }
  void accept(gdsl::rreil::id_visitor &v) const override;

  static std::shared_ptr<gdsl::rreil::id> from_symbol(analysis::symbol symb);
};
}
}
