#pragma once

#include <summy/analysis/domains/mempath.h>
#include <summy/analysis/domains/ptr_set.h>

namespace analysis {

struct mempath_assignment {
  mempath mp;
  ptr_set_t ptrs_immediate;

  bool operator<(const mempath_assignment &other) const;
  bool operator==(const mempath_assignment &other) const;
};
}

/*
 * Todo:
 *   - Model assignment of mempath to an immediate pointer
 *   - This assignment can be used as a key in the summary to context map
 */
