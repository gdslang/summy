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
public:
  domain_state() {

  }
  domain_state(domain_state &e) {
  }
  virtual ~domain_state() {
  }

  virtual domain_state *join(domain_state *other, size_t current_node) = 0;
  virtual domain_state *box(domain_state *other, size_t current_node) = 0;

  virtual bool operator>=(domain_state &other) = 0;

  virtual void put(std::ostream &out) = 0;
  friend std::ostream &operator<< (std::ostream &out, domain_state &_this);
};

inline std::ostream &operator<<(std::ostream &out, domain_state &_this) {
  _this.put(out);
  return out;
}

}
