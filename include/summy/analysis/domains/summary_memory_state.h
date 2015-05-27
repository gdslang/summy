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

namespace analysis {

struct field {
  size_t size;
  id_shared_t num_id;
};

/*
 * region: offset -> size, numeric id
 */
typedef std::map<int64_t, field> region_t;
/*
 * region_map: memory id -> memory region
 */
typedef std::map<id_shared_t, region_t, id_less_no_version> region_map_t;
/*
 * deref: numeric id -> memory region
 */
typedef region_map_t deref_t;

class summary_memory_state;
class managed_temporary {
private:
  summary_memory_state &_this;
  api::num_var *var;
public:
  managed_temporary(summary_memory_state &_this, api::num_var *var);
  managed_temporary(managed_temporary const&) = delete;
  ~managed_temporary();
  api::num_var *get_var() {
    return var;
  }
};

std::ostream &operator<<(std::ostream &out, field const &_this);

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
class summary_memory_state: public domain_state {
  friend class managed_temporary;
private:
  shared_ptr<static_memory> sm;

  numeric_state *child_state;
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
  region_t::iterator retrieve_kill(region_t &region, int64_t offset, size_t size);
  void topify(region_t &region, int64_t offset, size_t size);
  id_shared_t transVarReg(io_region io, int64_t offset, size_t size);
  id_shared_t transVar(id_shared_t var_id, int64_t offset, size_t size);
  id_shared_t transDeref(id_shared_t var_id, int64_t offset, size_t size);
  std::vector<field> transLERegFields(region_t &region, int64_t offset, size_t size);
  api::num_linear *assemble_fields(std::vector<field> fields);
  api::num_linear *transLEReg(io_region io, int64_t offset, size_t size);
  api::num_linear *transLE(id_shared_t var_id, int64_t offset, size_t size);
  api::num_linear *transLEInput(id_shared_t var_id, int64_t offset, size_t size);
public:
  summary_memory_state(shared_ptr<static_memory> sm, numeric_state *child_state, relation input, relation output) :
      sm(sm), child_state(child_state), input(input), output(output) {
  }
  /**
   * @param start_bottom: true => start value, false => bottom
   */
  summary_memory_state(shared_ptr<static_memory> sm, numeric_state *child_state, bool start_bottom);
  summary_memory_state(summary_memory_state const &o) :
      sm(o.sm), child_state(o.child_state->copy()), input(o.input), output(o.output) {
  }
  ~summary_memory_state() {
    delete child_state;
  }

  bool is_bottom();

  bool operator>=(domain_state const &other) const;

  summary_memory_state *join(domain_state *other, size_t current_node);
  summary_memory_state *widen(domain_state *other, size_t current_node);
  summary_memory_state *narrow(domain_state *other, size_t current_node);
  summary_memory_state *apply_summary(summary_memory_state *summary);

  typedef std::function<void(api::num_var*)> updater_t;
  void update_multiple(api::ptr_set_t aliases, regions_getter_t getter, size_t size, updater_t strong, updater_t weak);
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
  std::set<summy::vs_shared_t> queryPts(std::unique_ptr<managed_temporary> &address);
  api::ptr_set_t queryAls(gdsl::rreil::address *a);
//  region_t const& query_region(id_shared_t id);

  summary_memory_state *copy() const;

  static summary_memory_state *start_value(shared_ptr<static_memory> sm, numeric_state *start_num);
  static summary_memory_state *bottom(shared_ptr<static_memory> sm, numeric_state *bottom_num);

  struct memory_head {
    relation input;
    relation output;
  };
  static std::tuple<memory_head, numeric_state*, numeric_state*> compat(summary_memory_state const *a,
      summary_memory_state const *b);
};

}
