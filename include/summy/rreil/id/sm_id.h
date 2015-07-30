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
#include <memory>
#include <string>

using analysis::symbol;

namespace summy {
namespace rreil {

class sm_id : public gdsl::rreil::id {
private:
  std::string symbol;
  void *address;

  void put(std::ostream &out);

public:
  sm_id(std::string symbol, void *address) : symbol(symbol), address(address) {}
  ~sm_id();

  std::string get_symbol() {
    return symbol;
  }

  void *get_address() {
    return address;
  }

  bool operator==(gdsl::rreil::id &other) const;
  void accept(gdsl::rreil::id_visitor &v);

  static std::shared_ptr<gdsl::rreil::id> from_symbol(analysis::symbol symb);
};
}
}
