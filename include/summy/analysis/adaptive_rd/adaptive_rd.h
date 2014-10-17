/*
 * reaching_def.h
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/analysis.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/analysis/adaptive_rd/adaptive_rd_elem.h>
#include <vector>
#include <functional>
#include <set>
#include <ostream>
#include <cppgdsl/rreil/id/id.h>
#include <memory>

namespace analysis {
namespace adaptive_rd {

typedef std::vector<std::shared_ptr<adaptive_rd_elem>> state_t;

struct adaptive_rd_result : public ::analysis::analysis_result<state_t> {
  typedef std::vector<std::map<size_t, shared_ptr<adaptive_rd_elem>>> in_states_t;
  in_states_t in_states;

  adaptive_rd_result(state_t s, in_states_t in_states) : analysis_result(s), in_states(in_states) {
  }
};

class adaptive_rd: public analysis {
private:
  state_t state;
  ::analysis::liveness::liveness_result lv_result;

  virtual void init_constraints();
  virtual void init_dependants();
public:
  adaptive_rd(cfg::cfg *cfg, ::analysis::liveness::liveness_result lv_result);
  ~adaptive_rd();

  std::shared_ptr<lattice_elem> bottom();
  std::shared_ptr<lattice_elem> start_value();

  std::shared_ptr<lattice_elem> get(size_t node);
  void update(size_t node, std::shared_ptr<lattice_elem> state);
  adaptive_rd_result result();

  void put(std::ostream &out);
};

}  // namespace reaching_defs
}  // namespace analysis
