/*
 * fixpoint.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/cfg.h>
#include <summy/cfg/observer.h>
#include <summy/analysis/fp_priority_queue.h>
#include <set>
#include <map>
#include <vector>

namespace cfg {
class jd_manager;
}

namespace analysis {

class fp_analysis;

class fixpoint : public cfg::observer {
private:
  fp_analysis *analysis;
  std::set<size_t> seen;
  std::set<size_t> updated;
  cfg::jd_manager &jd_man;

  bool widening;

  fp_priority_queue worklist;

  std::map<size_t, size_t> node_iterations;
public:
  virtual ~fixpoint() {
  }

  fixpoint(class fp_analysis *analysis, cfg::jd_manager &jd_man, bool widening = true);

  void iterate();
  void notify(std::vector<cfg::update> const &updates);

  std::set<size_t> const &get_updated() {
    return updated;
  }

  /**
   * Get the maximum number of iterations needed for any
   * node
   */
  size_t max_iter();
};

}
