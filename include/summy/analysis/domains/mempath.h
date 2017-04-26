/*
 * mempath.h
 *
 *  Created on: Oct 29, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <cppgdsl/rreil/id/id.h>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <summy/analysis/domains/ptr_set.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <tuple>
#include <vector>

namespace analysis {

struct mp_result {
  std::vector<size_t> immediate_ptrs;
  size_t path_construction_errors;
};

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

  size_t _extract(std::experimental::optional<std::set<mempath>> &extracted,
    summary_memory_state *from, std::map<size_t, ptr_set_t> &aliases_from_immediate) const;

public:
  mempath(std::shared_ptr<gdsl::rreil::id> base, std::vector<step> path);

  bool operator<(const mempath &other) const;
  bool operator==(const mempath &other) const;

  /**
   * Propagate the described memory field from one state to another
   *
   * @return 'true' if the field is a requirement in 'from'
   */
  std::map<size_t, ptr_set_t> resolve(summary_memory_state *from) const;
  std::tuple<std::map<size_t, ptr_set_t>, ptr_set_t> split(
    std::map<size_t, ptr_set_t> aliases) const;
  //     std::experimental::optional<std::set<mempath>> extract(
  //       summary_memory_state *from, std::map<size_t, ptr_set_t> &aliases_from_immediate) const;
  void propagate(
    size_t path_length, ptr_set_t aliases_from_immediate, summary_memory_state *to) const;

  mp_result propagate(std::experimental::optional<std::set<mempath>> &extracted,
    summary_memory_state *from, summary_memory_state *to) const;

  static size_t from_aliases(
    std::set<mempath> &extracted, api::id_set_t aliases, summary_memory_state *state);

  friend std::ostream &operator<<(std::ostream &out, mempath const &_this);
};

std::ostream &operator<<(std::ostream &out, mempath const &_this);
}
