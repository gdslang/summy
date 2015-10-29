/*
 * memory_state.h
 *
 *  Created on: Mar 12, 2015
 *      Author: Julian Kranz
 */

#pragma once

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

namespace analysis {

/**
 * Memory domain state
 */
class memory_state: public domain_state, public memory_state_base {
private:
  shared_ptr<static_memory> sm;
  region_map_t regions;
  deref_t deref;

  typedef numeric_state*(numeric_state::*domopper_t) (domain_state *other, size_t current_node);
  memory_state *domop(domain_state *other, size_t current_node, domopper_t domopper);

  std::unique_ptr<managed_temporary> assign_temporary(int_t size, std::function<analysis::api::num_expr*(analysis::api::converter&)> cvc);

  void bottomify();
  region_t &dereference(id_shared_t id);
protected:
  void put(std::ostream &out) const;
  region_t &region(id_shared_t id);


  /*
   * Static memory
   */
  std::tuple<bool, void*> static_address(id_shared_t id);
  void initialize_static(region_t &region, void *address, size_t offset, size_t size);

  std::tuple<std::set<int64_t>, std::set<int64_t>> overlappings(summy::vs_finite *vs, int_t store_size);
  bool overlap_region(region_t &region, int64_t offset, size_t size);

  region_t::iterator retrieve_kill(region_t &region, int64_t offset, size_t size);
  void topify(region_t &region, int64_t offset, size_t size);
  id_shared_t transVarReg(region_t &region, int64_t offset, size_t size);
  id_shared_t transVar(id_shared_t var_id, int64_t offset, size_t size);
  api::num_linear *transLEReg(region_t &region, int64_t offset, size_t size);
  api::num_linear *transLE(id_shared_t var_id, int64_t offset, size_t size);
public:
  memory_state(shared_ptr<static_memory> sm, numeric_state *child_state, region_map_t regions, deref_t deref) :
      memory_state_base(child_state), sm(sm), regions(regions), deref(deref) {
  }
  /**
   * @param start_bottom: true => start value, false => bottom
   */
  memory_state(shared_ptr<static_memory> sm, numeric_state *child_state, bool start_bottom);
  memory_state(memory_state const &o) :
      memory_state_base(o.child_state->copy()), sm(o.sm), regions(o.regions), deref(o.deref) {
  }
  ~memory_state() {
    delete child_state;
  }

  bool is_bottom();

  bool operator>=(domain_state const &other) const;

  memory_state *join(domain_state *other, size_t current_node);
  memory_state *widen(domain_state *other, size_t current_node);
  memory_state *narrow(domain_state *other, size_t current_node);

  void update(gdsl::rreil::assign *assign);
  void update(gdsl::rreil::load *load);
  void update(gdsl::rreil::store *store);
  void assume(gdsl::rreil::sexpr *cond);
  void assume_not(gdsl::rreil::sexpr *cond);

  void cleanup();

  std::unique_ptr<managed_temporary> assign_temporary(gdsl::rreil::linear *l, int_t size);
  std::unique_ptr<managed_temporary> assign_temporary(gdsl::rreil::expr *e, int_t size);
  std::unique_ptr<managed_temporary> assign_temporary(gdsl::rreil::sexpr *se, int_t size);
  summy::vs_shared_t queryVal(gdsl::rreil::linear *l, size_t size);
  summy::vs_shared_t queryVal(gdsl::rreil::expr *e, size_t size);
  std::set<summy::vs_shared_t> queryPts(std::unique_ptr<managed_temporary> &address);
  ptr_set_t queryAls(gdsl::rreil::address *a);
  region_t const& query_region(id_shared_t id);

  memory_state *copy() const;

  static memory_state *start_value(shared_ptr<static_memory> sm, numeric_state *start_num);
  static memory_state *bottom(shared_ptr<static_memory> sm, numeric_state *bottom_num);

  struct memory_head {
    region_map_t regions;
    deref_t deref;
  };
  static std::tuple<memory_head, numeric_state*, numeric_state*> compat(memory_state const *a, memory_state const *b);
};

}
