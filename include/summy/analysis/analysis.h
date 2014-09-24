/*
 * analysis.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <queue>
#include <tuple>

namespace analysis {

class lattice_elem;

class analysis {
  /*
   * Hält cfg, vector von Zuständen
   */
public:
  /*
   * Eher node als edge als Parameter => constraint
   */
  virtual lattice_elem *eval(std::tuple<size_t, size_t> edge) = 0;
  virtual void update(size_t node, lattice_elem *state) = 0;
  virtual std::queue<size_t> initial() = 0;
};

}

