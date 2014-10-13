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
public:
  typedef std::function<std::shared_ptr<lattice_elem>()> constraint_t;
protected:
  cfg::cfg *cfg;
  std::vector<std::vector<constraint_t>> constraints;
  /*
   * Hält cfg, vector von Zuständen
   */
public:
  analysis(cfg::cfg *cfg) : cfg(cfg), constraints(cfg->node_count()) {
  }
  virtual ~analysis() {
  }

  virtual std::vector<constraint_t> constraints_at(size_t node) {
    return constraints[node];
  }
  virtual std::set<size_t> initial() = 0;

  virtual shared_ptr<lattice_elem> get(size_t node) = 0;
  virtual void update(size_t node, shared_ptr<lattice_elem> state) = 0;

  virtual std::set<size_t> dependants(size_t node_id) = 0;
};

}
