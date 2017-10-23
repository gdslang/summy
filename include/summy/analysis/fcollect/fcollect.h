/*
 * fcollect.h
 *
 *  Created on: Dec 11, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/fp_analysis.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/cfg.h>
#include <vector>
#include <functional>
#include <set>
#include <ostream>
#include <cppgdsl/rreil/id/id.h>
#include <memory>

namespace analysis {
namespace fcollect {

typedef std::set<size_t> state_t;

struct fcollect_result : public ::analysis::analysis_result<state_t> {
  fcollect_result(state_t &s) : analysis_result(s) {
  }
};

class fcollect: public fp_analysis {
private:
  state_t state;
  std::set<size_t> known_nodes;

  std::map<size_t, std::shared_ptr<domain_state>> transform(
    size_t from, size_t to, const ::cfg::edge *e, size_t from_ctx) override;
  virtual dependency gen_dependency(size_t from, size_t to) override;
  virtual void init_state() override;
public:
  fcollect(cfg::cfg *cfg);
  ~fcollect();

  std::shared_ptr<domain_state> get(size_t node) override;
  std::shared_ptr<domain_state> start_state(size_t) override;
  void update(analysis_node node, std::shared_ptr<domain_state> state) override;
  fcollect_result result();

  void put(std::ostream &out) override;
};

}
}  // namespace analysis
