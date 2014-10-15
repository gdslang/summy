/*
 * liveness.h
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/analysis.h>
#include <summy/analysis/liveness/lv_elem.h>
#include <memory>
#include <vector>

namespace analysis {
namespace liveness {

typedef std::vector<std::shared_ptr<lv_elem>> state_t;
typedef std::function<std::shared_ptr<lv_elem>()> constraint_t;

struct liveness_result : public ::analysis::analysis_result<state_t> {
  liveness_result(state_t s) : analysis_result(s) {
  }

  bool contains(size_t node_id, singleton_key_t sk, unsigned long long offset, unsigned long long size);
};

class liveness : public analysis {
private:
  state_t state;

  virtual void init_constraints();
  virtual void init_dependants();
public:
  liveness(cfg::cfg *cfg);
  ~liveness();

  shared_ptr<lattice_elem> bottom();

  shared_ptr<lattice_elem> get(size_t node);
  void update(size_t node, shared_ptr<lattice_elem> state);
  liveness_result result();

  void put(std::ostream &out);
};

}  // namespace reaching_defs
}  // namespace analysis
