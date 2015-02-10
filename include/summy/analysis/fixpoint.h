/*
 * fixpoint.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/cfg.h>
#include <summy/cfg/observer.h>
#include <set>
#include <vector>

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
};

class fp_analysis;

class fixpoint: cfg::observer {
private:
  fp_analysis *analysis;
  std::set<size_t> seen;
  std::set<size_t> updated;
public:
  virtual ~fixpoint() {
  }

  fixpoint(class fp_analysis *analysis) :
      analysis(analysis) {
  }

  void iterate();
  void notify(std::vector<cfg::update> const &updates);

  std::set<size_t> const &get_updated() {
    return updated;
  }
};

}
