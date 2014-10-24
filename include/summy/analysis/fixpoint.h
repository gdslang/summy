/*
 * fixpoint.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/cfg.h>
#include <set>
#include <vector>

namespace analysis {

class analysis;

class fixpoint {
private:
  analysis *analysis;
  std::set<size_t> seen;
public:
  virtual ~fixpoint() {
  }

  fixpoint(class analysis *analysis) : analysis(analysis) {
  }

  void iterate();
  void update(std::vector<::cfg::update> const &updates);
};

}
