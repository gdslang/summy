/*
 * caller.h
 *
 *  Created on: Feb 09, 2016
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/caller/caller_state.h>
#include <summy/analysis/fp_analysis.h>
#include <summy/cfg/cfg.h>
#include <vector>
#include <functional>
#include <set>
#include <map>
#include <ostream>
#include <cppgdsl/rreil/id/id.h>
#include <memory>

namespace analysis {
namespace caller {

typedef std::vector<std::shared_ptr<caller_state>> state_t;

struct caller_result : public ::analysis::analysis_result<state_t> {
  caller_result(state_t &s) : analysis_result(s) {
  }
};

class caller: public fp_analysis {
private:
  state_t state;

  std::map<size_t, std::shared_ptr<domain_state>> transform(
    size_t from, size_t to, const ::cfg::edge *e, size_t from_ctx);
  virtual dependency gen_dependency(size_t from, size_t to);
  virtual void init_state();
public:
  caller(cfg::cfg *cfg);
  ~caller();

  std::shared_ptr<caller_state> bottom();
  std::shared_ptr<domain_state> start_state(size_t node);

  std::shared_ptr<domain_state> get(size_t node);
  void update(size_t node, std::shared_ptr<domain_state> state);
  caller_result result();

  void put(std::ostream &out);
};

}  // namespace caller
}  // namespace analysis
