/*
 * ismt.h
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/liveness/liveness.h>
#include <summy/cfg/cfg.h>

namespace analysis {

class ismt {
private:
  cfg::cfg *cfg;
  liveness::liveness_result lv_result;

public:
  ismt(cfg::cfg *cfg, liveness::liveness_result lv_result) : cfg(cfg), lv_result(lv_result) {
  }
  void analyse(size_t from);
};

}  // namespace analysis
