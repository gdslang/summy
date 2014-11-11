/*
 * ismt.h
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/ismt/cvc_context.h>
#include <summy/analysis/ismt/smt_builder.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/cfg/cfg.h>

namespace analysis {

class ismt {
private:
  cfg::cfg *cfg;
  liveness::liveness_result lv_result;
  cvc_context context;
  smt_builder smtb;

public:
  ismt(cfg::cfg *cfg, liveness::liveness_result lv_result);
  ~ismt();
  void analyse(size_t from);
};

}  // namespace analysis
