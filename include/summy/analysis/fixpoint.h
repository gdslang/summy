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

class analysis;

class fixpoint : cfg::observer {
private:
  analysis *analysis;
  std::set<size_t> seen;
  std::set<size_t> updated;
public:
  virtual ~fixpoint() {
  }

  fixpoint(class analysis *analysis) : analysis(analysis) {
  }

  void iterate();
  void notify(std::vector<cfg::update> const &updates);

  std::set<size_t> const &get_updated() {
    return updated;
  }
};

}
