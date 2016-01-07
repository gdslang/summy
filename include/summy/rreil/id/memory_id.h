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
#include <experimental/optional>

namespace summy {
namespace rreil {

class memory_id : public gdsl::rreil::id {};

class allocation_memory_id : public memory_id {
private:
  void *allocation_site;

  void put(std::ostream &out);

  static size_t subclass_counter;

public:
  allocation_memory_id(void *allocation_site) : allocation_site(allocation_site) {}

  size_t get_subclass_counter() const {
    return subclass_counter;
  }

  void *get_allocation_site() {
    return allocation_site;
  }

  bool operator==(gdsl::rreil::id &other) const;
  bool operator<(id const &other) const;
  void accept(gdsl::rreil::id_visitor &v);
};

class ptr_memory_id : public memory_id {
private:
  std::shared_ptr<gdsl::rreil::id> inner;

  void put(std::ostream &out);

  static size_t subclass_counter;

public:
  ptr_memory_id(std::shared_ptr<gdsl::rreil::id> inner) : inner(inner) {}
  ~ptr_memory_id();

  size_t get_subclass_counter() const {
    return subclass_counter;
  }

  std::shared_ptr<gdsl::rreil::id> const &get_id() {
    return inner;
  }

  bool operator==(gdsl::rreil::id &other) const;
  bool operator<(id const &other) const;
  void accept(gdsl::rreil::id_visitor &v);
};
}
}
