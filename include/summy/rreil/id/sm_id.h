/*
 * sm_id.h
 *
 *  Created on: Apr 16, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id.h>
#include <cppgdsl/rreil/id/id_visitor.h>
#include <iostream>
#include <memory>
#include <string>

namespace summy {
namespace rreil {

class sm_id: public gdsl::rreil::id {
private:
  std::string symbol;

  void put(std::ostream &out);
public:
  sm_id(std::string symbol) :
    symbol(symbol) {
  }
  ~sm_id();

  std::string get_symbol() {
    return symbol;
  }

  bool operator== (gdsl::rreil::id &other);
  void accept(gdsl::rreil::id_visitor &v);

  static std::shared_ptr<gdsl::rreil::id> from_symbol(std::string symbol);
};

}
}
