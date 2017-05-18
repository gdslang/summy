/*
 * analysis.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include "analysis_result.h"
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <summy/analysis/analysis_visitor.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/observer.h>
#include <tuple>
#include <vector>

using std::shared_ptr;

namespace analysis {

using node_id_t = size_t;
using context_t = size_t;
  
struct analysis_node {
  node_id_t id;
  context_t context;

  bool operator<(analysis_node const &other) const {
    if(this->id < other.id)
      return true;
    else if(this->id > other.id)
      return false;
    else
      return this->context < other.context;
  }

  analysis_node(size_t id) : id(id), context(0) {}

  analysis_node(size_t id, size_t context) : id(id), context(context) {}
};

typedef std::function<std::experimental::optional<bool>(
  analysis_node const &, analysis_node const &)>
  node_compare_t;

class domain_state;

struct dependency {
  size_t source;
  size_t sink;
};

typedef std::function<std::map<size_t, std::shared_ptr<domain_state>>(size_t context)> constraint_t;

inline std::map<size_t, std::shared_ptr<domain_state>> default_context(
  std::shared_ptr<domain_state> state, size_t ctx = 0) {
  return {{ctx, state}};
}

inline std::map<size_t, std::shared_ptr<domain_state>> empty_context_map() {
  return std::map<size_t, std::shared_ptr<domain_state>>();
}

struct depdant_desc {
  std::set<size_t> context_free_deps;
  std::map<context_t, std::set<node_id_t>> context_deps;

  depdant_desc(depdant_desc const &other) = delete;
  depdant_desc operator=(depdant_desc const &other) = delete;

  depdant_desc(depdant_desc &&other)
      : context_free_deps(other.context_free_deps), context_deps(other.context_deps) {
    other.context_free_deps.clear();
    other.context_deps.clear();
  }
  
  depdant_desc& operator=(depdant_desc &&other) {
    this->context_free_deps = other.context_free_deps;
    this->context_deps = other.context_deps;
    other.context_free_deps.clear();
    other.context_deps.clear();
    return *this;
  }
  
  depdant_desc() {}

  depdant_desc(std::set<size_t> context_free_deps, std::map<size_t, std::set<size_t>> context_deps)
      : context_free_deps(context_free_deps), context_deps(context_deps) {}
};

class fp_analysis {
private:
  cfg::recorder rec;

public:
  typedef std::map<size_t, std::set<size_t>> dependants_t;

protected:
  cfg::cfg *cfg;

  std::map<size_t, std::map<size_t, constraint_t>> constraints;
  dependants_t _dependants;
  std::set<analysis_node> fixpoint_pending;

  virtual void add_constraint(size_t from, size_t to, const ::cfg::edge *e) = 0;
  virtual void remove_constraint(size_t from, size_t to) = 0;
  /*
   * Generate a dependency for an edge from node "from" to node "to"
   */
  virtual dependency gen_dependency(size_t from, size_t to) = 0;
  virtual void init_state() = 0;

  std::set<analysis_node> roots(std::set<analysis_node> const &all, const dependants_t &dep_dants);
  virtual void init_fixpoint_pending();
  void init();

public:
  cfg::cfg *get_cfg() {
    return cfg;
  }

  fp_analysis(cfg::cfg *cfg);
  virtual ~fp_analysis() {}

  void update(std::vector<::cfg::update> const &updates);
  void record_updates();
  bool record_stop_commit();

  virtual std::map<size_t, constraint_t> &constraints_at(size_t node) {
    return constraints[node];
  }
  virtual std::set<analysis_node> pending() {
    return fixpoint_pending;
  }
  void clear_pending();

  virtual shared_ptr<domain_state> get(size_t node) = 0;
  virtual std::map<size_t, shared_ptr<domain_state>> get_ctxful(size_t node) {
    return {{0, get(node)}};
  }
  virtual void update(analysis_node node, shared_ptr<domain_state> state) = 0;


  virtual depdant_desc dependants(size_t node_id) {
    return {_dependants[node_id], std::map<size_t, std::set<size_t>>()};
  }
  void assert_dependency(dependency dep);

  /**
   * This method calculates a set of nodes that got dirty during constraint
   * evaluation. This allows the analysis to tell the fixpoint engine to re-
   * evaluate additional nodes.
   */
  virtual depdant_desc dirty_nodes() {
    return depdant_desc();
  }

  virtual node_compare_t get_fixpoint_node_comparer();

  virtual void check_consistency() {}

  virtual void ref(size_t node, std::experimental::optional<size_t> count) {}
  virtual void unref(size_t node) {}

  virtual void accept(analysis_visitor &v) {
    v.visit(this);
  }

  virtual void put(std::ostream &out) = 0;
  friend std::ostream &operator<<(std::ostream &out, fp_analysis &_this);
};

std::ostream &operator<<(std::ostream &out, fp_analysis &_this);
}
