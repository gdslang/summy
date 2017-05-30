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
  char v;

public:
  fcollect_state(char v) : v(v)  {
  }

  /*
   * Copy constructor
   */
  fcollect_state(fcollect_state &e) {
  }

  virtual fcollect_state *join(::analysis::domain_state *other, size_t current_node) {
    auto other_casted = dynamic_cast<fcollect_state*>(other);
    return new fcollect_state(other_casted->v | this->v);
  }
  virtual fcollect_state *narrow(::analysis::domain_state *other, size_t current_node) {
    auto other_casted = dynamic_cast<fcollect_state*>(other);
    return new fcollect_state(other_casted->v & this->v);
  }
  virtual fcollect_state *widen(::analysis::domain_state *other, size_t current_node) {
    return join(other, current_node);
  }

  virtual bool operator>=(::analysis::domain_state const &other) const {
    auto const& other_casted = dynamic_cast<fcollect_state const&>(other);
    return v >= other_casted.v;
  }

  virtual void put(std::ostream &out) const {
    out << "[fcollect_state]";
  }
};

}
}
