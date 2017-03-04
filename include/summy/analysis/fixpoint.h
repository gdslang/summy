/*
 * fixpoint.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <map>
#include <set>
#include <summy/analysis/fp_priority_queue.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/observer.h>
#include <vector>
#include <ctime>

namespace cfg {
class jd_manager;
}

namespace analysis {

class fp_analysis;

class fixpoint : public cfg::observer {
private:
  fp_analysis *analysis;
  std::set<analysis_node> seen;
  std::set<size_t> updated;
  cfg::jd_manager &jd_man;

  bool ref_management;
  bool widening;

  fp_priority_queue worklist;

  std::set<size_t> machine_addresses;
  std::map<size_t, size_t> node_iterations;
  size_t max_its;

  std::time_t construct_time;
public:
  virtual ~fixpoint() {}

  fixpoint(class fp_analysis *analysis, cfg::jd_manager &jd_man, bool ref_management,
    bool widening = true);

  void iterate();
  void notify(std::vector<cfg::update> const &updates);

  std::set<size_t> const &get_updated() {
    return updated;
  }
  
  /*
   * Statistics stuff
   */

  /**
   * Get the maximum number of iterations needed for any
   * node
   */
  size_t max_iter();
  void print_distribution();
  void print_distribution_total();
  double avg_iteration_count();
  
  size_t analyzed_addresses();
  
  void print_hot_addresses();
};
}
