/*
 * reaching_def.h
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/analysis.h>
#include <vector>
#include <set>
#include <cppgdsl/rreil/id/id.h>

namespace analysis {
namespace reaching_defs {

class reaching_defs : analysis {
public:
  typedef std::vector<std::set<gdsl::rreil::id*>> state_t;
private:
  state_t state;
public:
  lattice_elem *eval(size_t node);
  std::queue<size_t> initial();

  lattice_elem *get(size_t node);
  void update(size_t node, lattice_elem *state);

  std::set<size_t> dependants(size_t node_id);
};

}  // namespace reaching_defs
}  // namespace analysis
