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
#include <summy/value_set/value_set.h>
#include <cppgdsl/rreil/id/id.h>
#include <set>
#include <map>
#include <tuple>
#include <memory>
#include <iostream>

namespace analysis {

typedef std::set<size_t> callers_t;

class global_state: public domain_state {
private:
  summary_memory_state *mstate;
  summy::vs_shared_t f_addr;
public:
  summary_memory_state *get_mstate() {
    return mstate;
  }

  summy::vs_shared_t get_f_addr() {
    return f_addr;
  }

  void set_f_addr(summy::vs_shared_t f_addr) {
    this->f_addr = f_addr;
  }

  global_state(summary_memory_state *mstate, summy::vs_shared_t f_addr) :
      mstate(mstate), f_addr(f_addr) {
  }

  global_state(global_state const& o) :
      mstate(o.mstate->copy()), f_addr(o.f_addr) {
  }

  ~global_state();

  virtual global_state *join(::analysis::domain_state *other, size_t current_node);
  virtual global_state *narrow(::analysis::domain_state *other, size_t current_node);
  virtual global_state *widen(::analysis::domain_state *other, size_t current_node);

  virtual bool operator>=(::analysis::domain_state const &other) const;

  void check_consistency();

  virtual void put(std::ostream &out) const;
};

}
