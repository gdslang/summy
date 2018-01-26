#pragma once

#include <iostream>
#include <set>
#include <summy/analysis/domains/mempath.h>
#include <summy/analysis/domains/ptr_set.h>

namespace analysis {

struct mempath_assignment {
  mempath mp;
  ptr immediate;

  mempath_assignment(mempath mp, ptr immediate) : mp(mp), immediate(immediate) {}

  void propagate(summary_memory_state *to) const;

  bool operator<(const mempath_assignment &other) const;
  bool operator==(const mempath_assignment &other) const;
};

inline std::ostream &operator<<(std::ostream &out, mempath_assignment const &_this) {
  out << _this.mp << " := " << _this.immediate;
  return out;
};

struct mempath_aliasing {
  /**
   * A set of alias sets; each set in this set contains a set of pointers
   * that alias.
   */
  std::set<std::set<mempath>> aliasings;

  const std::set<std::set<mempath>> &get_aliasings() {
    return aliasings;
  }

  mempath_aliasing() {}

  void add_aliasing(std::set<mempath> aliasing) {
    aliasings.insert(aliasing);
  }

  bool operator<(const mempath_aliasing &other) const;
  bool operator==(const mempath_aliasing &other) const;
};

struct tabulation_key {
  std::set<mempath_assignment> fptr_answers;
  std::set<mempath_aliasing> aliasing_answers;

  tabulation_key(
    std::set<mempath_assignment> fptr_answers, std::set<mempath_aliasing> aliasing_answers)
      : fptr_answers(fptr_answers), aliasing_answers(aliasing_answers) {}

  tabulation_key(
    std::set<mempath_assignment> fptr_answers)
      : fptr_answers(fptr_answers), aliasing_answers() {}

  bool operator<(const tabulation_key &other) const;
  bool operator==(const tabulation_key &other) const;
};

} // namespace analysis

/*
 * Idea:
 *   - Model assignment of mempath to an immediate pointer
 *   - This assignment can be used as a key in the summary to context map
 */
