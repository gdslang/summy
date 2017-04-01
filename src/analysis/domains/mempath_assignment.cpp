#include <summy/analysis/domains/mempath_assignment.h>

namespace analysis {

bool mempath_assignment::operator<(const mempath_assignment &other) const {
  if(mp < other.mp)
    return true;
  else if(other.mp < mp)
    return false;
  else
    return ptrs_immediate < other.ptrs_immediate;
}

bool mempath_assignment::operator==(const mempath_assignment &other) const {
  return mp == other.mp && ptrs_immediate == other.ptrs_immediate;
}

}
