/*
 * summary_memory_state.h
 *
 *  Created on: Apr 21, 2015
 *      Author: Julian Kranz
 */

#pragma once

/*
 * Todo: Combine with memory state
 */

#include <summy/analysis/domain_state.h>
#include <summy/analysis/domains/numeric/numeric_state.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/domains/memstate_util.h>
#include <summy/value_set/value_set.h>
#include <summy/analysis/util.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/static_memory.h>

#include <cppgdsl/rreil/statement/assign.h>
#include <memory>
#include <map>
#include <set>
#include <tuple>
#include <functional>
#include <experimental/optional>

namespace analysis {

struct relation {
  region_map_t regions;
  deref_t deref;

  region_map_t &get_regions() {
    return regions;
  }

  deref_t &get_deref() {
    return deref;
  }

  void clear();
};

struct io_region {
  region_t &in_r;
  region_t &out_r;

  field &insert(numeric_state *child_state, int64_t offset, size_t size);
};

/**
 * Summary-based memory domain state
 */
class summary_memory_state: public domain_state, public memory_state_base {
private:
  shared_ptr<static_memory> sm;

  relation input;
  relation output;

  typedef numeric_state*(numeric_state::*domopper_t)(domain_state *other, size_t current_node);
  summary_memory_state *domop(domain_state *other, size_t current_node, domopper_t domopper);

  std::unique_ptr<managed_temporary> assign_temporary(int_t size,
      std::function<analysis::api::num_expr*(analysis::api::converter&)> cvc);

  typedef region_map_t&(relation::*regions_getter_t)();
  io_region region_by_id(regions_getter_t getter, id_shared_t id);

  void bottomify();
  io_region dereference(id_shared_t id);
protected:
  void put(std::ostream &out) const;
//  region_t &region(id_shared_t id);

  /*
   * Static memory
   */
  std::tuple<bool, void*> static_address(id_shared_t id);
  void initialize_static(io_region io, void *address, size_t offset, size_t size);

  std::tuple<std::set<int64_t>, std::set<int64_t>> overlappings(summy::vs_finite *vs, int_t store_size);
  bool overlap_region(region_t &region, int64_t offset, size_t size);

//  region_t merge_memory(id_shared_t addr_a, region_t &r);
//  region_t merge_memory(id_shared_t addr_a, id_shared_t addr_b);
  struct rt_result_t {
    bool conflict;
    region_t::iterator region_it;
  };
  rt_result_t retrieve_kill(region_t &region, int64_t offset, size_t size, bool handle_conflict);
  region_t::iterator retrieve_kill(region_t &region, int64_t offset, size_t size);
  void topify(region_t &region, int64_t offset, size_t size);
  std::experimental::optional<id_shared_t> transVarReg(io_region io, int64_t offset, size_t size, bool handle_conflict);
  id_shared_t transVarReg(io_region io, int64_t offset, size_t size);
  id_shared_t transVar(id_shared_t var_id, int64_t offset, size_t size);
  id_shared_t transDeref(id_shared_t var_id, int64_t offset, size_t size);
  std::vector<field> transLERegFields(region_t &region, int64_t offset, size_t size);
  api::num_linear *assemble_fields(std::vector<field> fields);
  api::num_linear *transLEReg(io_region io, int64_t offset, size_t size);
  api::num_linear *transLE(regions_getter_t rget, id_shared_t var_id, int64_t offset, size_t size);
  api::num_linear *transLE(id_shared_t var_id, int64_t offset, size_t size);
  api::num_linear *transLEInput(id_shared_t var_id, int64_t offset, size_t size);
public:
  summary_memory_state(shared_ptr<static_memory> sm, numeric_state *child_state, relation input, relation output) :
      memory_state_base(child_state), sm(sm), input(input), output(output) {
  }
  /**
   * @param start_bottom: true => start value, false => bottom
   */
  summary_memory_state(shared_ptr<static_memory> sm, numeric_state *child_state, bool start_bottom);
  summary_memory_state(summary_memory_state const &o) :
      memory_state_base(o.child_state->copy()), sm(o.sm), input(o.input), output(o.output) {
  }
  ~summary_memory_state() {
    delete child_state;
  }

  bool is_bottom() const;

  bool operator>=(domain_state const &other) const;

  summary_memory_state *join(domain_state *other, size_t current_node);
  summary_memory_state *widen(domain_state *other, size_t current_node);
  summary_memory_state *narrow(domain_state *other, size_t current_node);
  summary_memory_state *apply_summary(summary_memory_state *summary);

  typedef std::function<void(api::num_var*)> updater_t;
  void update_multiple(api::ptr_set_t aliases, regions_getter_t getter, size_t size, updater_t strong, updater_t weak,
      bool handle_conflicts = true);
  void store(api::ptr_set_t aliases, size_t size, api::num_expr *rhs);

  void update(gdsl::rreil::assign *assign);
  void update(gdsl::rreil::load *load);
  void update(gdsl::rreil::store *store);
  void assume(gdsl::rreil::sexpr *cond);
  void assume_not(gdsl::rreil::sexpr *cond);

  void cleanup();
  void project(api::num_vars *vars);
  api::num_vars *vars_relations();
  api::num_vars *vars();

  std::unique_ptr<managed_temporary> assign_temporary(gdsl::rreil::linear *l, int_t size);
  std::unique_ptr<managed_temporary> assign_temporary(gdsl::rreil::expr *e, int_t size);
  std::unique_ptr<managed_temporary> assign_temporary(gdsl::rreil::sexpr *se, int_t size);
  summy::vs_shared_t queryVal(gdsl::rreil::linear *l, size_t size);
  summy::vs_shared_t queryVal(gdsl::rreil::expr *e, size_t size);
  api::num_linear *dereference(api::num_var *v, int64_t offset, size_t size);
  std::set<summy::vs_shared_t> queryPts(std::unique_ptr<managed_temporary> &address);
  api::ptr_set_t queryAls(gdsl::rreil::address *a);
  api::ptr_set_t queryAls(api::num_var *a);
  region_t const& query_region_output(id_shared_t id);

  summary_memory_state *copy() const;

  static summary_memory_state *start_value(shared_ptr<static_memory> sm, numeric_state *start_num);
  static summary_memory_state *bottom(shared_ptr<static_memory> sm, numeric_state *bottom_num);

  /**
   * This function tries to establish a mapping between pointers by structurally matching
   * the input of the given summaries. If a pointer is only found in one of the summaries,
   * the respective pointer is added to the other summary. This way, all pointers are matched
   * as long as there are no conflicts, that is, partially overlapping fields.
   *
   * @return pair of variables that correspond to each other in the respective memory states
   */
  static num_var_pairs_t matchPointers(relation &a_in, relation &a_out, numeric_state *a_n, relation &b_in,
      relation &b_out, numeric_state *b_n);
  struct memory_head {
    relation input;
    relation output;
  };
  static std::tuple<memory_head, numeric_state*, numeric_state*> compat(summary_memory_state const *a,
      summary_memory_state const *b);
};

}
