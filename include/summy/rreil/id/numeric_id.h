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
#include <string>
#include <assert.h>
#include <optional>

namespace summy {
namespace rreil {

class copy_visitor;

class numeric_id : public gdsl::rreil::id {
  friend class copy_visitor;

private:
  int_t counter;
  std::optional<std::string> name;
  std::optional<bool> input;

  void put(std::ostream &out) const override;

  static size_t subclass_counter;

  numeric_id(int_t counter, std::optional<std::string> name, std::optional<bool> input)
      : counter(counter), name(name), input(input) {}

public:
  numeric_id(numeric_id const& o) : counter(o.counter), name(o.name), input(o.input) {
  }
  ~numeric_id();

  size_t get_subclass_counter() const override {
    return subclass_counter;
  }

  int_t get_counter() const {
    return counter;
  }

  std::optional<std::string> get_name() const {
    return name;
  }

  std::optional<bool> get_input() const {
    return input;
  }

  bool operator==(gdsl::rreil::id const &other) const override;
  bool operator<(id const &other) const override;
  std::unique_ptr<gdsl::rreil::id> copy() const override;
  void accept(gdsl::rreil::id_visitor &v) const override;

  static std::shared_ptr<gdsl::rreil::id> generate(
    std::optional<std::string> name = std::nullopt,
    std::optional<bool> input = std::nullopt);
  static std::shared_ptr<gdsl::rreil::id> generate(std::string reg_name, int64_t offset, size_t size, bool input);
};
}
}
