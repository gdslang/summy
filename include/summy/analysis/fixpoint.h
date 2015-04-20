/*
 * fixpoint.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/cfg.h>
#include <summy/cfg/jd_manager.h>
#include <summy/cfg/observer.h>
#include <set>
#include <vector>

using cfg::jd_manager;

namespace analysis {

class fp_priority_queue {
private:
  std::set<size_t> inner;
public:
  fp_priority_queue() {
  }
  fp_priority_queue(std::set<size_t> init) :
      inner(init) {
  }

  void push(size_t value);
  size_t pop();
  bool empty();
  void clear();
};

class fp_analysis;

class fixpoint : cfg::observer {
private:
  fp_analysis *analysis;
  std::set<size_t> seen;
  std::set<size_t> updated;
  jd_manager &jd_man;
public:
  virtual ~fixpoint() {
  }

  fixpoint(class fp_analysis *analysis, jd_manager &jd_man) :
      analysis(analysis), jd_man(jd_man) {
  }

  void iterate();
  void notify(std::vector<cfg::update> const &updates);

  std::set<size_t> const &get_updated() {
    return updated;
  }
};

}
