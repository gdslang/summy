/*
 * state.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <memory>
#include <iostream>

using std::shared_ptr;

namespace analysis {

class domain_state {
protected:
  virtual void put(std::ostream &out) const = 0;
public:
  domain_state() {

  }
  domain_state(domain_state &e) {
  }
  virtual ~domain_state() {
  }

  virtual domain_state *join(domain_state *other, size_t current_node) = 0;
  virtual domain_state *box(domain_state *other, size_t current_node) = 0;

  virtual bool operator>=(domain_state const &other) const = 0;
  bool operator<=(domain_state const &other) const;
  bool operator==(domain_state const &other) const;

  friend std::ostream &operator<< (std::ostream &out, domain_state const &_this);
};

inline std::ostream &operator<<(std::ostream &out, domain_state const &_this) {
  _this.put(out);
  return out;
}

}
