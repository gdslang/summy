/*
 * state.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once

namespace analysis {

class lattice_elem {
public:
  virtual ~lattice_elem() {
  }

  virtual lattice_elem *lub(lattice_elem *other) = 0;
  virtual bool operator>(lattice_elem &other) = 0;
};

}
