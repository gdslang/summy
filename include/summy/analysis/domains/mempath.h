/*
 * mempath.h
 *
 *  Created on: Oct 29, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/ptr_set.h>
#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <vector>
#include <set>
#include <memory>
#include <iostream>

namespace analysis {

class mempath {
public:
  struct step {
    int64_t offset;
    size_t size;

    step(int64_t offset, size_t size) : offset(offset), size(size) {}

    bool operator<(const step &other) const;
  };

private:
  std::shared_ptr<gdsl::rreil::id> base;
  std::vector<step> path;

  int compare_to(const mempath &other) const;

public:
  mempath(std::shared_ptr<gdsl::rreil::id> base, std::vector<step> path);

  bool operator<(const mempath &other) const;
  bool operator==(const mempath &other) const;

  /**
   * Propagate the described memory field from one state to another
   *
   * @return 'true' if the field is a requirement in 'from'
   */
  bool propagate(summary_memory_state *from, summary_memory_state *to);

  static std::set<mempath> from_aliases(api::id_set_t aliases, summary_memory_state *state);

  friend std::ostream &operator<<(std::ostream &out, mempath const &_this);
};

std::ostream &operator<<(std::ostream &out, mempath const &_this);
}
