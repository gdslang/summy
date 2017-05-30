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
#include <summy/analysis/fp_analysis.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/analysis/reaching_defs/rd_state.h>
#include <vector>

namespace analysis {
namespace reaching_defs {

typedef std::vector<std::shared_ptr<rd_state>> state_t;
typedef ::analysis::analysis_result<state_t> reaching_defs_result_t;

class reaching_defs : public fp_analysis {
private:
  state_t state;
  ::analysis::liveness::liveness_result lv_result;

  std::map<size_t, std::shared_ptr<domain_state>> transform(
    size_t from, size_t to, const ::cfg::edge *e, size_t from_ctx);
  virtual dependency gen_dependency(size_t from, size_t to);
  virtual void init_state();

public:
  reaching_defs(cfg::cfg *cfg, ::analysis::liveness::liveness_result lv_result);
  ~reaching_defs();

  std::shared_ptr<domain_state> bottom();
  std::shared_ptr<domain_state> start_state(size_t);

  std::shared_ptr<domain_state> get(size_t node);
  void update(analysis_node node, std::shared_ptr<domain_state> state);
  reaching_defs_result_t result();

  void put(std::ostream &out);
};

} // namespace reaching_defs
} // namespace analysis
