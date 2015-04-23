/*
 * global_analysis.h
 *
 *  Created on: Apr 23, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/fp_analysis.h>
#include <summy/analysis/static_memory.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <summy/analysis/global_analysis/global_state.h>
#include <memory>
#include <vector>
#include <set>
#include <map>

namespace analysis {

typedef std::map<size_t, std::shared_ptr<global_state>> state_t;

struct global_analysis_result : public ::analysis::analysis_result<state_t> {
  global_analysis_result(state_t &s) :
      analysis_result(s) {
  }
};

class global_analysis : public fp_analysis {
private:
  std::shared_ptr<static_memory> sm;
  state_t state;

  void add_constraint(size_t from, size_t to, const ::cfg::edge *e);
  void remove_constraint(size_t from, size_t to);
  dependency gen_dependency(size_t from, size_t to);
  void init_state();
public:
  global_analysis(cfg::cfg *cfg, std::shared_ptr<static_memory> sm);
  global_analysis(cfg::cfg *cfg);
  ~global_analysis();

  std::shared_ptr<domain_state> bottom();
  std::shared_ptr<domain_state> start_value();

  std::shared_ptr<domain_state> get(size_t node);
  void update(size_t node, shared_ptr<domain_state> state);
  global_analysis_result result();

  void put(std::ostream &out);
};

}  // namespace analysis
