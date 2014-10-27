/*
 * ssa.h
 *
 *  Created on: Oct 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/big_step/big_step.h>
#include <summy/cfg/cfg.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/analysis/adaptive_rd/adaptive_rd.h>
#include <summy/analysis/fixpoint.h>
#include <summy/transformers/ssa/phi_inserter.h>
#include <summy/transformers/ssa/renamer.h>
#include <vector>

/*
 * Todo: class ssa: run analysis, transformations, register for future graph changes
 */

class ssa : public big_step {
private:
  analysis::liveness::liveness l;
  analysis::fixpoint fpl;

  analysis::adaptive_rd::adaptive_rd r;
  analysis::fixpoint fpr;

  phi_inserter pi;
  renamer ren;

public:
  ssa(cfg::cfg &cfg);

  void transduce();
  void notify(std::vector<cfg::update> const &updates);
};
