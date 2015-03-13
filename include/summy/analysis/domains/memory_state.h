/*
 * memory_state.h
 *
 *  Created on: Mar 12, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/domain_state.h>
#include <summy/analysis/domains/numeric/numeric_state.h>
#include <summy/value_set/value_set.h>
#include <summy/analysis/util.h>
#include <memory>
#include <map>

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
typedef std::map<id_shared_t, region_t> region_map_t;
/*
 * deref: numeric id -> memory region
 */
typedef std::map<id_shared_t, region_t> deref_t;

/**
 * Memory domain state
 */
class memory_state : public domain_state {
private:
  std::shared_ptr<numeric_state> child_state;
  region_map_t regions;
  deref_t deref;
protected:
  void put(std::ostream &out) const;
public:
  memory_state(std::shared_ptr<numeric_state> child_state) : child_state(child_state) {
  }

  bool operator>=(domain_state const &other) const;

  memory_state *join(domain_state *other, size_t current_node);
  memory_state *widen(domain_state *other, size_t current_node);
  memory_state *narrow(domain_state *other, size_t current_node);
  memory_state *box(domain_state *other, size_t current_node);
};

}
