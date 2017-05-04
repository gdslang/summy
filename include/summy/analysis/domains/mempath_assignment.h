#pragma once

#include <summy/analysis/domains/mempath.h>
#include <summy/analysis/domains/ptr_set.h>

namespace analysis {

struct mempath_assignment {
  mempath mp;
  ptr immediate;
  
  mempath_assignment(mempath mp, ptr immediate) : mp(mp), immediate(immediate) {
  }
  
  void propagate(summary_memory_state *to) const;

  bool operator<(const mempath_assignment &other) const;
  bool operator==(const mempath_assignment &other) const;
};
}

/*
 * Idea:
 *   - Model assignment of mempath to an immediate pointer
 *   - This assignment can be used as a key in the summary to context map
 */
