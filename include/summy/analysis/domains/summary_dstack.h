/*
 * summary_dstack.h
 *
 *  Created on: Apr 22, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/fp_analysis.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <summy/analysis/global_analysis/global_state.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <summy/value_set/value_set.h>
#include <memory>
#include <vector>
#include <set>
#include <map>

namespace analysis {

typedef std::vector<std::shared_ptr<global_state>> state_t;

struct summary_dstack_result : public ::analysis::analysis_result<state_t> {
  summary_dstack_result(state_t &s) :
      analysis_result(s) {
  }
};

typedef std::shared_ptr<summary_memory_state> summary_t;

struct function_desc {
  summary_t summary;
  /*
   * Minimal call stack size
   */
  size_t min_calls_sz;
  /*
   * Node id of head node
   */
  size_t head_id;

  function_desc(summary_t summary, size_t min_calls_sz, size_t head_id) :
      summary(summary), min_calls_sz(min_calls_sz), head_id(head_id) {
  }
};

class summary_dstack : public fp_analysis {
private:
  std::shared_ptr<static_memory> sm;
  std::map<void*, function_desc> function_desc_map;
  state_t state;

  bool unpack_f_addr(void *&r, summy::vs_shared_t f_addr);

  void add_constraint(size_t from, size_t to, const ::cfg::edge *e);
  void remove_constraint(size_t from, size_t to);
  dependency gen_dependency(size_t from, size_t to);
  void init_state();
public:
  summary_dstack(cfg::cfg *cfg, std::shared_ptr<static_memory> sm);
  summary_dstack(cfg::cfg *cfg);
  ~summary_dstack();

  summary_memory_state *sms_bottom();
  summary_memory_state *sms_top();
  std::shared_ptr<domain_state> bottom();
  std::shared_ptr<domain_state> start_value();
  std::shared_ptr<domain_state> start_value(summy::vs_shared_t f_addr, callers_t callers);

  std::shared_ptr<domain_state> get(size_t node);
  void update(size_t node, shared_ptr<domain_state> state);
  summary_dstack_result result();

  node_compare_t get_fixpoint_node_comparer();

  void put(std::ostream &out);
};

}  // namespace analysis
