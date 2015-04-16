/*
 * dstack.h
 *
 *  Created on: Mar 17, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/fp_analysis.h>
#include <summy/analysis/domains/memory_state.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <memory>
#include <vector>
#include <set>

namespace analysis {

typedef std::vector<std::shared_ptr<memory_state>> state_t;
//typedef std::function<std::shared_ptr<lv_state>()> constraint_t;
//typedef std::map<size_t, std::vector<singleton_t>> newly_live_t;
//typedef cfg::edge_payload_map_t<bool> edge_bool_map_t;


struct dstack_result : public ::analysis::analysis_result<state_t> {
  dstack_result(state_t &s) :
      analysis_result(s) {
  }
};

class dstack : public fp_analysis {
private:
  std::shared_ptr<static_memory> sm;
  state_t state;

  void add_constraint(size_t from, size_t to, const ::cfg::edge *e);
  void remove_constraint(size_t from, size_t to);
  dependency gen_dependency(size_t from, size_t to);
  void init_state();
public:
  dstack(cfg::cfg *cfg, std::shared_ptr<static_memory> sm);
  dstack(cfg::cfg *cfg);
  ~dstack();

  std::shared_ptr<domain_state> bottom();
  std::shared_ptr<domain_state> start_value();

  std::shared_ptr<domain_state> get(size_t node);
  void update(size_t node, shared_ptr<domain_state> state);
  dstack_result result();

  void put(std::ostream &out);
};

}  // namespace analysis
