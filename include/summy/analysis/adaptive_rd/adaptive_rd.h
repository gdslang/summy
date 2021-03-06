/*
 * reaching_def.h
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id.h>
#include <functional>
#include <memory>
#include <ostream>
#include <set>
#include <summy/analysis/adaptive_rd/adaptive_rd_state.h>
#include <summy/analysis/fp_analysis.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <vector>

namespace analysis {
namespace adaptive_rd {

typedef std::vector<std::shared_ptr<adaptive_rd_state>> state_t;
typedef std::vector<std::map<size_t, shared_ptr<adaptive_rd_state>>> in_states_t;

struct adaptive_rd_result : public ::analysis::analysis_result<state_t> {
  in_states_t &in_states;

  adaptive_rd_result(state_t &s, in_states_t &in_states) : analysis_result(s), in_states(in_states) {}
};

class adaptive_rd : public fp_analysis {
private:
  state_t state;
  in_states_t in_states;
  liveness::liveness_result lv_result;

  std::map<size_t, std::shared_ptr<domain_state>> transform(
    size_t from, size_t to, const ::cfg::edge *e, size_t from_ctx) override;
  virtual dependency gen_dependency(size_t from, size_t to) override;
  virtual void init_state() override;

public:
  adaptive_rd(cfg::cfg *cfg, liveness::liveness_result lv_result);
  ~adaptive_rd();

  std::shared_ptr<domain_state> bottom();
  std::shared_ptr<domain_state> start_state(size_t) override;

  std::shared_ptr<domain_state> get(size_t node) override;
  void update(analysis_node node, std::shared_ptr<domain_state> state) override;
  adaptive_rd_result result();

  void put(std::ostream &out) override;
};

} // namespace reaching_defs
} // namespace analysis
