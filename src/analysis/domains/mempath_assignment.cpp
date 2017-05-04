#include <summy/analysis/domains/mempath_assignment.h>

namespace analysis {

void mempath_assignment::propagate(summary_memory_state *to) const {
  mp.propagate(mp.get_path().size(), {immediate}, to);
}

bool mempath_assignment::operator<(const mempath_assignment &other) const {
  if(mp < other.mp)
    return true;
  else if(other.mp < mp)
    return false;
  else
    return immediate < other.immediate;
}

bool mempath_assignment::operator==(const mempath_assignment &other) const {
  return mp == other.mp && immediate == other.immediate;
}
}
