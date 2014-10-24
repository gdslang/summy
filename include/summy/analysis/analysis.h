/*
 * analysis.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/cfg.h>
#include <queue>
#include <set>
#include <map>
#include <vector>
#include <tuple>
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

struct dependency {
  size_t source;
  size_t sink;
};

class analysis {
public:
  typedef std::function<std::shared_ptr<lattice_elem>()> constraint_t;
  typedef std::map<size_t, std::set<size_t>> dependants_t;
protected:
  cfg::cfg *cfg;
  std::map<size_t, std::map<size_t, constraint_t>> constraints;
  dependants_t _dependants;
  std::set<size_t> fixpoint_pending;

  virtual void add_constraint(size_t from, size_t to, const ::cfg::edge *e) = 0;
  virtual void remove_constraint(size_t from, size_t to) = 0;
  /*
   * Generate a dependency for an edge from node "from" to node "to"
   */
  virtual dependency gen_dependency(size_t from, size_t to) = 0;
  virtual void init_state() = 0;

  virtual void init_fixpoint_pending();
  void init();
public:
  analysis(cfg::cfg *cfg);
  virtual ~analysis() {
  }

  void update(std::vector<::cfg::update> const &updates);

  virtual std::map<size_t, constraint_t> &constraints_at(size_t node) {
    return constraints[node];
  }
  virtual std::set<size_t> pending() {
    return fixpoint_pending;
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
