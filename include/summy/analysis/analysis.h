/*
 * analysis.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <queue>
#include <set>
#include <map>
#include <tuple>
#include <summy/cfg/cfg.h>
#include <memory>
#include <iostream>

using std::shared_ptr;

namespace analysis {

class lattice_elem;

template<typename STATE_T>
struct analysis_result {
  STATE_T &result;

  analysis_result(STATE_T &result) : result(result) {
  }
};

class analysis {
public:
  typedef std::function<std::shared_ptr<lattice_elem>()> constraint_t;
protected:
  cfg::cfg *cfg;
  std::vector<std::map<size_t, constraint_t>> constraints;
  std::vector<std::set<size_t>> _dependants;
  std::set<size_t> fixpoint_initial;

  virtual void init_constraints() = 0;
  virtual void init_dependants() = 0;
  virtual void init_fixpoint_initial();
  void init();
public:
  analysis(cfg::cfg *cfg);
  virtual ~analysis() {
  }

  virtual std::map<size_t, constraint_t> &constraints_at(size_t node) {
    return constraints[node];
  }
  virtual std::set<size_t> initial() {
    return fixpoint_initial;
  }

  virtual shared_ptr<lattice_elem> get(size_t node) = 0;
  virtual void update(size_t node, shared_ptr<lattice_elem> state) = 0;

  virtual std::set<size_t> dependants(size_t node_id) {
    return _dependants[node_id];
  }

  virtual void put(std::ostream &out) = 0;
  friend std::ostream &operator<< (std::ostream &out, analysis &_this);
};

std::ostream &operator<<(std::ostream &out, analysis &_this);

}
