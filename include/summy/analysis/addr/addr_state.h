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
namespace addr {

typedef std::function<size_t(size_t)> get_next_virt_t;

struct path_virts_s {
  static const size_t n = 4;
  static const size_t size_singleton_bits = sizeof(uint64_t) * 8;

  uint64_t data[n];

  path_virts_s(uint64_t a);
  path_virts_s() : path_virts_s(1) {}
  path_virts_s(const path_virts_s &path_virts);

  uint64_t &operator[](size_t index);
  uint64_t const &operator[](size_t index) const;

  bool operator <=(path_virts_s const& other) const;

  path_virts_s insert(size_t virt) const;
  path_virts_s _union(path_virts_s const& virts_other) const;
};

std::ostream &operator<<(std::ostream &out, path_virts_s const &_this);

struct node_addr {
  size_t machine;
  size_t virt;

  node_addr(size_t machine, size_t virt) : machine(machine), virt(virt) {}

  bool operator<(node_addr const &other) const;
  bool operator<=(node_addr const &other) const;
  bool operator==(node_addr const &other) const;
};

std::ostream &operator<<(std::ostream &out, node_addr const &_this);

class addr_state : public domain_state {
private:
  const path_virts_s path_virts;
  const std::experimental::optional<node_addr> address;
  const get_next_virt_t get_next_virt;

  std::experimental::optional<size_t> next_virt_value;

  addr_state *domop(::analysis::domain_state *other);

  addr_state(node_addr address, path_virts_s path_virts, get_next_virt_t get_next_virt)
      : domain_state(), path_virts(path_virts), address(address), get_next_virt(get_next_virt) {}

public:
  std::experimental::optional<node_addr> const &get_address() {
    return address;
  }

  addr_state(node_addr address, get_next_virt_t get_next_virt)
      : domain_state(), path_virts(path_virts_s()), address(address), get_next_virt(get_next_virt) {}

  /*
   * Copy constructor
   */
  addr_state(addr_state &e)
      : domain_state(e), path_virts(e.path_virts), address(e.address), get_next_virt(e.get_next_virt) {}

  /**
   * Bottom constructor
   */
  addr_state(get_next_virt_t get_next_virt)
      : domain_state(), path_virts(0), address(std::experimental::nullopt), get_next_virt(get_next_virt) {}

  virtual addr_state *join(::analysis::domain_state *other, size_t current_node);
  virtual addr_state *narrow(::analysis::domain_state *other, size_t current_node);
  virtual addr_state *widen(::analysis::domain_state *other, size_t current_node);

  addr_state *next_virt();

  virtual bool operator>=(::analysis::domain_state const &other) const;

  virtual void put(std::ostream &out) const;
};
}
}
