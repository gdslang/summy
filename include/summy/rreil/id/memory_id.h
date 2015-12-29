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

  static size_t subclass_counter;
public:
  memory_id(int_t deref, std::shared_ptr<gdsl::rreil::id> inner) :
      deref(deref), inner(inner) {
  }
  ~memory_id();

  size_t get_subclass_counter() const {
    return subclass_counter;
  }

  int_t get_deref() {
    return deref;
  }

  std::shared_ptr<gdsl::rreil::id> const& get_id() {
    return inner;
  }

  bool operator==(gdsl::rreil::id &other) const;
  bool operator<(id const& other) const;
  void accept(gdsl::rreil::id_visitor &v);
};

}
}
