/*
 * reaching_def.h
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/analysis.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/analysis/reaching_defs/rd_elem.h>
#include <vector>
#include <functional>
#include <set>
#include <ostream>
#include <cppgdsl/rreil/id/id.h>
#include <memory>

namespace analysis {
namespace reaching_defs {

typedef std::vector<std::shared_ptr<rd_elem>> state_t;
typedef ::analysis::analysis_result<state_t> reaching_defs_result_t;

class reaching_defs: public analysis {
private:
  state_t state;
  ::analysis::liveness::liveness_result lv_result;

  virtual void add_constraint(size_t from, size_t to, const ::cfg::edge *e);
  virtual void remove_constraint(size_t from, size_t to);
  virtual void add_dependency(size_t from, size_t to);
  virtual void remove_dependency(size_t from, size_t to);
public:
  reaching_defs(cfg::cfg *cfg, ::analysis::liveness::liveness_result lv_result);
  ~reaching_defs();

  std::shared_ptr<lattice_elem> bottom();
  std::shared_ptr<lattice_elem> start_value();

  std::shared_ptr<lattice_elem> get(size_t node);
  void update(size_t node, std::shared_ptr<lattice_elem> state);
  reaching_defs_result_t result();

  void put(std::ostream &out);
};

}  // namespace reaching_defs
}  // namespace analysis
