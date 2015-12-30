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
#include <experimental/optional>

namespace summy {
namespace rreil {

class copy_visitor;

class numeric_id : public gdsl::rreil::id {
  friend class copy_visitor;

private:
  int_t counter;
  std::experimental::optional<std::string> name;
  std::experimental::optional<bool> input;

  void put(std::ostream &out);

  static size_t subclass_counter;

  numeric_id(int_t counter, std::experimental::optional<std::string> name, std::experimental::optional<bool> input)
      : counter(counter), name(name), input(input) {}

public:
  ~numeric_id();

  size_t get_subclass_counter() const {
    return subclass_counter;
  }

  int_t get_counter() {
    return counter;
  }

  std::experimental::optional<std::string> get_name() {
    return name;
  }

  std::experimental::optional<bool> get_input() {
    return input;
  }

  bool operator==(gdsl::rreil::id &other) const;
  bool operator<(id const &other) const;
  void accept(gdsl::rreil::id_visitor &v);

  static std::shared_ptr<gdsl::rreil::id> generate(
    std::experimental::optional<std::string> name = std::experimental::nullopt,
    std::experimental::optional<bool> input = std::experimental::nullopt);
  static std::shared_ptr<gdsl::rreil::id> generate(std::string reg_name, int64_t offset, size_t size, bool input);
};
}
}
