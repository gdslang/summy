/*
 * ptr_set.h
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <cppgdsl/rreil/id/id.h>
#include <summy/value_set/value_set.h>
#include <summy/analysis/util.h>
#include <set>
#include <memory>

namespace anaylsis {
namespace api {

struct ptr {
  std::shared_ptr<gdsl::rreil::id> id;
  summy::vs_shared_t offset;

  ptr(std::shared_ptr<gdsl::rreil::id> id, summy::vs_shared_t offset) :
      id(id), offset(offset) {
  }

  bool operator <(struct ptr other);
};

typedef std::set<ptr> ptr_set_t;

}
}
