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
#include <memory>

using std::shared_ptr;

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

  virtual shared_ptr<lattice_elem> eval(size_t node) = 0;
  virtual std::set<size_t> initial() = 0;

  virtual shared_ptr<lattice_elem> get(size_t node) = 0;
  virtual void update(size_t node, shared_ptr<lattice_elem> state) = 0;

  virtual std::set<size_t> dependants(size_t node_id) = 0;
};

}

