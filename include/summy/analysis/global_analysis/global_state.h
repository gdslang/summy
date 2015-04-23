/*
 * global_state.h
 *
 *  Created on: Apr 23, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/domain_state.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <summy/analysis/util.h>
#include <cppgdsl/rreil/id/id.h>
#include <set>
#include <map>
#include <tuple>
#include <memory>
#include <iostream>

namespace analysis {

class global_state: public domain_state {
private:
  shared_ptr<summary_memory_state> returned;
  shared_ptr<summary_memory_state> consecutive;
public:
  global_state(shared_ptr<summary_memory_state> returned, shared_ptr<summary_memory_state> consecutive) :
      returned(returned), consecutive(consecutive) {
  }

  virtual global_state *join(::analysis::domain_state *other, size_t current_node);
  virtual global_state *narrow(::analysis::domain_state *other, size_t current_node);
  virtual global_state *widen(::analysis::domain_state *other, size_t current_node);

  global_state *apply_summary();

  virtual bool operator>=(::analysis::domain_state const &other) const;

  virtual void put(std::ostream &out) const;
};

}
