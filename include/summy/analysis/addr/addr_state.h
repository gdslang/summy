/*
 * addr_state.h
 *
 *  Created on: Dec 22, 2015
 *      Author: Julian Kranz
 */

#pragma once

#pragma once

#include <summy/analysis/domain_state.h>

#include <set>
#include <map>
#include <tuple>
#include <ostream>
#include <memory>
#include <experimental/optional>

namespace analysis {
namespace addr {

class addr_state : public domain_state {
private:
  const std::experimental::optional<size_t> address;

  addr_state *domop(::analysis::domain_state *other);
public:
  std::experimental::optional<size_t> const& get_address() {
    return address;
  }

  addr_state(size_t address) :
      domain_state(), address(address) {
  }

  /*
   * Copy constructor
   */
  addr_state(addr_state &e) :
      domain_state(e), address(e.address) {
  }

  /**
   * Bottom constructor
   */
  addr_state() :
      domain_state(), address(std::experimental::nullopt) {
  }

  virtual addr_state *join(::analysis::domain_state *other, size_t current_node);
  virtual addr_state *narrow(::analysis::domain_state *other, size_t current_node);
  virtual addr_state *widen(::analysis::domain_state *other, size_t current_node);

  virtual bool operator>=(::analysis::domain_state const &other) const;

  virtual void put(std::ostream &out) const;
};

}
}
