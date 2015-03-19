/*
 * memory_id.h
 *
 *  Created on: Mar 18, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id.h>
#include <cppgdsl/rreil/id/id_visitor.h>
#include <iostream>
#include <memory>

namespace summy {
namespace rreil {

class memory_id: public gdsl::rreil::id {
private:
  int_t deref;
  std::shared_ptr<gdsl::rreil::id> inner;

  void put(std::ostream &out);
public:
  memory_id(int_t deref, std::shared_ptr<gdsl::rreil::id> inner) :
      deref(deref), inner(inner) {
  }
  ~memory_id();

  int_t get_deref() {
    return deref;
  }

  std::shared_ptr<gdsl::rreil::id> get_id() {
    return inner;
  }

  bool operator==(gdsl::rreil::id &other);
  void accept(gdsl::rreil::id_visitor &v);
};

}
}
