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
  std::unique_ptr<gdsl::rreil::id> id;
  size_t version;

  void put(std::ostream &out) const override;

  static size_t subclass_counter;
public:
  ssa_id(std::unique_ptr<gdsl::rreil::id> id, size_t version) :
      id(std::move(id)), version(version) {
  }
  ssa_id(ssa_id const& o) : id(o.id->copy()), version(o.version) {}

  size_t get_subclass_counter() const override {
    return subclass_counter;
  }

  gdsl::rreil::id const& get_id() const {
    return *id;
  }

  size_t get_version() const {
    return version;
  }

  bool operator== (gdsl::rreil::id const &other) const override;
  bool operator<(class id const& other) const override;
  std::unique_ptr<gdsl::rreil::id> copy() const override;
  void accept(gdsl::rreil::id_visitor &v) const override;
};

}
}
