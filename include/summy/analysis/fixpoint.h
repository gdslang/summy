/*
 * fixpoint.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/cfg.h>
#include <set>

namespace analysis {

class analysis;

class fixpoint {
private:
//  cfg::cfg *cfg;
  analysis *analysis;
  std::set<size_t> seen;
public:
  virtual ~fixpoint() {
  }

  fixpoint(/*cfg::cfg *cfg, */class analysis *analysis) : /*cfg(cfg),*/ analysis(analysis) {
  }

  void iterate();
};

}
