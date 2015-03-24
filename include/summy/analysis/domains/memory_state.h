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
#include <summy/value_set/value_set.h>
#include <summy/analysis/util.h>
#include <cppgdsl/rreil/statement/assign.h>
#include <memory>
#include <map>
#include <tuple>

namespace analysis {

struct field {
  size_t size;
  id_shared_t num_id;
};

/*
 * region: offset -> size, numeric id
 */
typedef std::map<size_t, field> region_t;
/*
 * region_map: memory id -> memory region
 */
typedef std::map<id_shared_t, region_t, id_less_no_version> region_map_t;
/*
 * deref: numeric id -> memory region
 */
typedef region_map_t deref_t;

/**
 * Memory domain state
 */
class memory_state: public domain_state {
private:
  numeric_state *child_state;
  region_map_t regions;
  deref_t deref;

  struct temp_s {
    memory_state &_this;
    api::num_var *var;
    ~temp_s();
  };
  temp_s assign_address(gdsl::rreil::address *a);

  region_t &dereference(id_shared_t id);
protected:
  void put(std::ostream &out) const;
  region_t &region(id_shared_t id);

  id_shared_t transVar(id_shared_t var_id, size_t offset, size_t size);
  api::num_linear *transLE(id_shared_t var_id, size_t offset, size_t size);
public:
  memory_state(numeric_state *child_state, region_map_t regions, deref_t deref) :
      child_state(child_state), regions(regions), deref(deref) {
  }
  /**
   * @param start_bottom: true => start value, false => bottom
   */
  memory_state(numeric_state *child_state, bool start_bottom);
  memory_state(memory_state const &o) :
      child_state(o.child_state->copy()), regions(o.regions), deref(o.deref) {
  }
  ~memory_state() {
    delete child_state;
  }

  bool is_bottom();

  bool operator>=(domain_state const &other) const;

  memory_state *join(domain_state *other, size_t current_node);
  memory_state *widen(domain_state *other, size_t current_node);
  memory_state *narrow(domain_state *other, size_t current_node);
  memory_state *box(domain_state *other, size_t current_node);

  void update(gdsl::rreil::assign *assign);
  void update(gdsl::rreil::load *load);
  void update(gdsl::rreil::store *store);

  memory_state *copy() const;

  static memory_state *start_value(numeric_state *start_num);
  static memory_state *bottom(numeric_state *bottom_num);

  struct memory_head {
    region_map_t regions;
    deref_t deref;
  };
  static std::tuple<memory_head, numeric_state*, numeric_state*> compat(memory_state const *a, memory_state const *b);
};

}
