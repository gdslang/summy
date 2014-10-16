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

class lattice_elem {
public:
  lattice_elem() {

  }
  lattice_elem(lattice_elem &e) {
  }
  virtual ~lattice_elem() {
  }

  virtual lattice_elem *lub(lattice_elem *other, size_t current_node) = 0;
  virtual bool operator>=(lattice_elem &other) = 0;

  virtual void put(std::ostream &out) = 0;
  friend std::ostream &operator<< (std::ostream &out, lattice_elem &_this);
};

inline std::ostream &operator<<(std::ostream &out, lattice_elem &_this) {
  _this.put(out);
  return out;
}

}
