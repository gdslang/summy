/*
 * analysis_id.h
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

class analysis_id: public gdsl::rreil::id {
private:
  size_t numeric_id;

  void put(std::ostream &out);
public:
  analysis_id(size_t numeric_id) :
      numeric_id(numeric_id) {
  }
  ~analysis_id();

  size_t get_numeric_id() {
    return numeric_id;
  }

  bool operator== (gdsl::rreil::id &other);
  void accept(gdsl::rreil::id_visitor &v);

  std::shared_ptr<gdsl::rreil::id> generate();
};

}
}
