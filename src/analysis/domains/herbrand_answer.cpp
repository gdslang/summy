#include <summy/analysis/domains/herbrand_answer.h>

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

bool mempath_aliasing::operator<(const mempath_aliasing &other) const {
  return aliasings < other.aliasings;
}

bool mempath_aliasing::operator==(const mempath_aliasing &other) const {
  return aliasings == other.aliasings;
}

bool tabulation_key::operator<(const tabulation_key &other) const {
  /*
   * This is pretty ineffiecient now as it compares the same set multiple
   * times.
   */
  if(fptr_answers < other.fptr_answers)
    return true;
  else if(other.fptr_answers < fptr_answers)
    return false;
  else
    return aliasing_answers < other.aliasing_answers;
}

bool tabulation_key::operator==(const tabulation_key &other) const {
  return fptr_answers == other.fptr_answers && other.aliasing_answers == other.aliasing_answers;
}

} // namespace analysis
