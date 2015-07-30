/*
 * special_ptr.h
 *
 *  Created on: Jul 27, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id.h>
#include <cppgdsl/rreil/id/id_visitor.h>
#include <summy/analysis/static_memory.h>
#include <iostream>
#include <memory>
#include <string>

using analysis::symbol;

namespace summy {
namespace rreil {

enum special_ptr_kind { NULL_PTR, BAD_PTR };

class special_ptr : public gdsl::rreil::id {
private:
  special_ptr_kind kind;

  void put(std::ostream &out);

public:
  special_ptr(special_ptr_kind kind) : kind(kind) {}
  ~special_ptr();

  special_ptr_kind get_kind() {
    return kind;
  }

  bool operator==(gdsl::rreil::id &other) const;
  void accept(gdsl::rreil::id_visitor &v);

  static std::shared_ptr<gdsl::rreil::id> _nullptr;
  static std::shared_ptr<gdsl::rreil::id> badptr;
};
}
}
