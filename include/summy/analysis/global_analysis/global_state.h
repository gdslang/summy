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

typedef std::set<size_t> callers_t;

class global_state: public domain_state {
private:
  summary_memory_state *mstate;
  size_t fstart_id;
  callers_t callers;
public:
  summary_memory_state *get_mstate() {
    return mstate;
  }

  size_t get_fstart_id() {
    return fstart_id;
  }

  callers_t &get_callers() {
    return callers;
  }

  global_state(summary_memory_state *mstate, size_t fstart_id, callers_t callers) :
      mstate(mstate), fstart_id(fstart_id), callers(callers) {
  }

  global_state(global_state const& o) :
      mstate(o.mstate->copy()), fstart_id(o.fstart_id), callers(o.callers) {
  }

  virtual global_state *join(::analysis::domain_state *other, size_t current_node);
  virtual global_state *narrow(::analysis::domain_state *other, size_t current_node);
  virtual global_state *widen(::analysis::domain_state *other, size_t current_node);

  virtual bool operator>=(::analysis::domain_state const &other) const;

  virtual void put(std::ostream &out) const;
};

}
