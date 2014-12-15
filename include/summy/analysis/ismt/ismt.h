/*
 * ismt.h
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/ismt/cvc_context.h>
#include <summy/analysis/ismt/smt_builder.h>
#include <summy/analysis/ismt/smt_def_builder.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/cfg/cfg.h>
#include <iosfwd>
#include <map>
#include <set>
#include <cvc4/cvc4.h>

namespace analysis {

typedef std::map<cfg::edge_id, std::set<size_t>> ismt_edge_ass_t;

class ismt {
private:
  cfg::cfg *cfg;
  liveness::liveness_result lv_result;
  adaptive_rd::adaptive_rd_result rd_result;
  cvc_context context;
  smt_builder smtb;
  smt_def_builder smt_defb;

  std::map<size_t, std::map<size_t, CVC4::Expr>> state;
public:
  ismt(cfg::cfg *cfg, liveness::liveness_result lv_result, adaptive_rd::adaptive_rd_result rd_result);
  ~ismt();
  ismt_edge_ass_t analyse(size_t from);
  void dot(std::ostream &stream);
};

}  // namespace analysis
