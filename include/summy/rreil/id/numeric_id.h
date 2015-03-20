/*
 * numeric_id.h
 *
 *  Created on: Mar 12, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id.h>
#include <cppgdsl/rreil/id/id_visitor.h>
#include <iostream>
#include <memory>

namespace summy {
namespace rreil {

class numeric_id: public gdsl::rreil::id {
private:
  int_t counter;

  void put(std::ostream &out);
public:
  numeric_id(int_t counter) :
    counter(counter) {
  }
  ~numeric_id();

  int_t get_counter() {
    return counter;
  }

  bool operator== (gdsl::rreil::id &other);
  void accept(gdsl::rreil::id_visitor &v);

  static std::shared_ptr<gdsl::rreil::id> generate();
};

}
}