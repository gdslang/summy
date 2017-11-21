/*
 * caller_state.h
 *
 *  Created on: Feb 09, 2016
 *      Author: Julian Kranz
 */

#pragma once

#pragma once

#include <summy/analysis/domain_state.h>

#include <set>
#include <ostream>
#include <memory>
#include <stdint.h>
#include <optional>

namespace analysis {
namespace caller {

class caller_state : public domain_state {
private:
  const std::set<size_t> callers;

  caller_state(std::set<size_t> callers) : domain_state(), callers(callers) {}

public:
  std::set<size_t> const &get_callers() {
    return callers;
  }

  /*
   * Copy constructor
   */
  caller_state(caller_state &e)
      : domain_state(e), callers(e.callers) {}

  /**
   * Bottom constructor
   */
  caller_state()
      : domain_state() {}

  virtual caller_state *join(::analysis::domain_state *other, size_t current_node);
  virtual caller_state *narrow(::analysis::domain_state *other, size_t current_node);
  virtual caller_state *widen(::analysis::domain_state *other, size_t current_node);

  caller_state *add_caller(size_t caller);

  virtual bool operator>=(::analysis::domain_state const &other) const;

  virtual void put(std::ostream &out) const;
};
}
}
