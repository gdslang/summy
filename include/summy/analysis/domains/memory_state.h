/*
 * memory_state.h
 *
 *  Created on: Mar 12, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/domain_state.h>
#include <summy/analysis/domains/numeric/numeric_state.h>
#include <summy/value_set/value_set.h>
#include <summy/analysis/util.h>
#include <memory>
#include <map>

namespace analysis {

struct field {
  size_t size;

};

//typedef map<id_shared_t, >

/**
 * Memory domain state
 */
class memory_state : public domain_state {
private:
  std::shared_ptr<numeric_state> child_state;
protected:
  void put(std::ostream &out) const;
public:
  memory_state(std::shared_ptr<numeric_state> child_state) : child_state(child_state) {
  }

  bool operator>=(domain_state const &other) const;

  memory_state *join(domain_state *other, size_t current_node);
  memory_state *widen(domain_state *other, size_t current_node);
  memory_state *narrow(domain_state *other, size_t current_node);
  memory_state *box(domain_state *other, size_t current_node);
};

}
