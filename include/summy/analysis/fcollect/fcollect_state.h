/*
 * fcollect_state.h
 *
 *  Created on: Dec 11, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/domain_state.h>

#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/util.h>
#include <set>
#include <map>
#include <tuple>
#include <ostream>
#include <memory>

namespace analysis {
namespace fcollect {

class fcollect_state : public domain_state {
private:

public:
  fcollect_state()  {
  }

  /*
   * Copy constructor
   */
  fcollect_state(fcollect_state &e) {
  }

  virtual fcollect_state *join(::analysis::domain_state *other, size_t current_node) {
    return new fcollect_state();
  }
  virtual fcollect_state *narrow(::analysis::domain_state *other, size_t current_node) {
    return new fcollect_state();
  }
  virtual fcollect_state *widen(::analysis::domain_state *other, size_t current_node) {
    return new fcollect_state();

  }

  virtual bool operator>=(::analysis::domain_state const &other) const {
    return true;
  }

  virtual void put(std::ostream &out) const {
    out << "[fcollect_state]";
  }
};

}
}
