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
#include <stdint.h>
#include <experimental/optional>

namespace analysis {
namespace addr_machine {

class addr_machine_state : public domain_state {
private:
  const std::experimental::optional<size_t> address;

  addr_machine_state *domop(::analysis::domain_state *other);

public:
  addr_machine_state(size_t address)
      : domain_state(), address(address) {}

  std::experimental::optional<size_t> const &get_address() {
    return address;
  }

  /*
   * Copy constructor
   */
  addr_machine_state(addr_machine_state &e)
      : domain_state(e), address(e.address) {}

  /**
   * Bottom constructor
   */
  addr_machine_state()
      : domain_state(), address(std::experimental::nullopt) {}

  virtual addr_machine_state *join(::analysis::domain_state *other, size_t current_node);
  virtual addr_machine_state *narrow(::analysis::domain_state *other, size_t current_node);
  virtual addr_machine_state *widen(::analysis::domain_state *other, size_t current_node);

  virtual bool operator>=(::analysis::domain_state const &other) const;

  virtual void put(std::ostream &out) const;
};
}
}
