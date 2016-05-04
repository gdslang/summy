/*
 * summary_dstack.h
 *
 *  Created on: Apr 22, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/analysis_visitor.h>
#include <summy/analysis/caller/caller.h>
#include <summy/analysis/domains/mempath.h>
#include <summy/analysis/domains/summary_dstack_stubs.h>
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
#include <unordered_map>
#include <experimental/optional>

namespace analysis {

typedef std::vector<std::shared_ptr<global_state>> state_t;

struct summary_dstack_result : public ::analysis::analysis_result<state_t> {
  summary_dstack_result(state_t &s) : analysis_result(s) {}
};

typedef std::shared_ptr<summary_memory_state> summary_t;

struct function_desc {
  std::set<size_t> summary_nodes;
  /*
   * Minimal call stack size
   */
  size_t min_calls_sz;
  /*
   * Node id of head node
   */
  size_t head_id;
  /*
   * Field requirements (function pointers)
   */
  std::set<mempath> field_reqs;

  function_desc(size_t min_calls_sz, size_t head_id) : min_calls_sz(min_calls_sz), head_id(head_id) {}
};

class summary_dstack : public fp_analysis {
private:
  std::shared_ptr<static_memory> sm;
  bool warnings;
  std::map<void *, function_desc> function_desc_map;
  std::set<size_t> _dirty_nodes;
//  caller::caller caller_analysis;
  state_t state;

  std::set<size_t> erased;

  std::unordered_map<size_t, std::experimental::optional<size_t>> ref_map;

  summary_dstack_stubs stubs;
  std::experimental::optional<summary_t> get_stub(void *address, size_t node);

  std::set<size_t> get_callers(std::shared_ptr<global_state> state);
  std::set<size_t> get_function_heads(std::shared_ptr<global_state> state);
  static std::set<void *> unpack_f_addrs(summy::vs_shared_t f_addr);
  //  void propagate_reqs(void *f_addr, std::set<mempath> &field_reqs_new);

  void propagate_reqs(std::set<mempath> field_reqs_new, void *f_addr);
  void add_constraint(size_t from, size_t to, const ::cfg::edge *e);
  void remove_constraint(size_t from, size_t to);
  dependency gen_dependency(size_t from, size_t to);
  void init_state(summy::vs_shared_t f_addr);
  void init_state();

public:
  summary_dstack(cfg::cfg *cfg, std::shared_ptr<static_memory> sm, bool warnings, std::set<size_t> const &f_starts);
  [[deprecated]] summary_dstack(cfg::cfg *cfg, std::shared_ptr<static_memory> sm, bool warnings);
  summary_dstack(cfg::cfg *cfg, bool warnings);
  ~summary_dstack();

  summary_memory_state *sms_bottom();
  summary_memory_state *sms_top();
  static summary_memory_state *sms_bottom(std::shared_ptr<static_memory> sm, bool warnings);
  static summary_memory_state *sms_top(std::shared_ptr<static_memory> sm, bool warnings);
  std::shared_ptr<domain_state> bottom();
  std::shared_ptr<domain_state> start_value(summy::vs_shared_t f_addr);

  std::shared_ptr<domain_state> get(size_t node);
  std::shared_ptr<global_state> get_sub(size_t node);
  void update(size_t node, shared_ptr<domain_state> state);
  summary_dstack_result result();

  node_compare_t get_fixpoint_node_comparer();

  std::set<size_t> dirty_nodes();

  virtual void check_consistency();

  void ref(size_t node, std::experimental::optional<size_t> count);
  void unref(size_t node);

  std::experimental::optional<size_t> get_lowest_function_address(size_t node_id);
  void print_callstack(size_t node_id);

  virtual void accept(analysis_visitor &v) {
    v.visit(this);
  }

  void put(std::ostream &out);
};

} // namespace analysis
