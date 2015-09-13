/*
 * ssa_id.h
 *
 *  Created on: Oct 20, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id.h>
#include <cppgdsl/rreil/id/id_visitor.h>
#include <iostream>

namespace summy {
namespace rreil {

class ssa_id: public gdsl::rreil::id {
private:
  gdsl::rreil::id *id;
  size_t version;

  void put(std::ostream &out);

  static size_t subclass_counter;
public:
  ssa_id(gdsl::rreil::id *id, size_t version) :
      id(id), version(version) {
  }
  ~ssa_id();

  size_t get_subclass_counter() {
    return subclass_counter;
  }

  gdsl::rreil::id *get_id() {
    return id;
  }

  size_t get_version() {
    return version;
  }

  bool operator== (gdsl::rreil::id &other) const;
  void accept(gdsl::rreil::id_visitor &v);
};

}
}
