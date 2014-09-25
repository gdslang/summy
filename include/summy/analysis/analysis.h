/*
 * analysis.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <queue>
#include <set>
#include <tuple>
#include <summy/cfg/cfg.h>

namespace analysis {

class lattice_elem;

class analysis {
protected:
  cfg::cfg *cfg;
  /*
   * Hält cfg, vector von Zuständen
   */
public:
  analysis(cfg::cfg *cfg) : cfg(cfg) {
  }
  virtual ~analysis() {
  }

  virtual lattice_elem *eval(size_t node) = 0;
  virtual std::queue<size_t> initial() = 0;

  virtual lattice_elem *get(size_t node) = 0;
  virtual void update(size_t node, lattice_elem *state) = 0;

  virtual std::set<size_t> dependants(size_t node_id) = 0;
};

}

