/*
 * memstate_util.h
 *
 *  Created on: May 27, 2015
 *      Author: jucs
 */

#pragma once
#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/domains/numeric/numeric_state.h>
#include <map>
#include <iostream>

namespace analysis {

struct field {
  size_t size;
  id_shared_t num_id;
};

std::ostream &operator<<(std::ostream &out, field const &_this);

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

class managed_temporary;
class memory_state_base {
  friend class managed_temporary;
protected:
  numeric_state *child_state;
public:
  memory_state_base(numeric_state *child_state) : child_state(child_state) {
  }
};

class managed_temporary {
private:
  memory_state_base &_this;
  api::num_var *var;
public:
  managed_temporary(memory_state_base &_this, api::num_var *var);
  managed_temporary(managed_temporary const&) = delete;
  ~managed_temporary();
  api::num_var *get_var() {
    return var;
  }
};

}
