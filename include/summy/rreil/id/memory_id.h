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
#include <assert.h>
#include <memory>
#include <optional>

namespace summy {
namespace rreil {

class memory_id : public gdsl::rreil::id {};

class allocation_memory_id : public memory_id {
private:
  size_t allocation_site;

  void put(std::ostream &out) const override;

  static size_t subclass_counter;

public:
  allocation_memory_id(size_t allocation_site) : allocation_site(allocation_site) {}
  allocation_memory_id(allocation_memory_id const &o) : allocation_site(o.allocation_site) {}

  size_t get_subclass_counter() const override {
    return subclass_counter;
  }

  size_t get_allocation_site() const {
    return allocation_site;
  }

  bool operator==(gdsl::rreil::id const &other) const override;
  bool operator<(id const &other) const override;
  std::unique_ptr<gdsl::rreil::id> copy() const override;
  void accept(gdsl::rreil::id_visitor &v) const override;
};

class ptr_memory_id : public memory_id {
private:
  std::unique_ptr<gdsl::rreil::id> inner;

  void put(std::ostream &out) const override;

  static size_t subclass_counter;

public:
  ptr_memory_id(std::unique_ptr<gdsl::rreil::id> inner) : inner(std::move(inner)) {}
  ptr_memory_id(ptr_memory_id const &o) : inner(o.inner->copy()) {
  }
  ~ptr_memory_id();

  size_t get_subclass_counter() const override {
    return subclass_counter;
  }

  gdsl::rreil::id const &get_id() const {
    return *inner;
  }

  bool operator==(gdsl::rreil::id const &other) const override;
  bool operator<(id const &other) const override;
  std::unique_ptr<gdsl::rreil::id> copy() const override;
  void accept(gdsl::rreil::id_visitor &v) const override;
};
}
}
