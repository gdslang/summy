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
  virtual void lub(lattice_elem *other);
};

}
