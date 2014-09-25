/*
 * reaching_def.h
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/analysis.h>
#include <summy/analysis/reaching_defs/lattice_elem.h>
#include <vector>
#include <functional>
#include <set>
#include <cppgdsl/rreil/id/id.h>

namespace analysis {
namespace reaching_defs {

class reaching_defs : public analysis {
public:
  typedef std::vector<::analysis::reaching_defs::lattice_elem*> state_t;
private:
  state_t state;
  std::vector<std::function<lattice_elem*()>> constraints;
public:
  reaching_defs(cfg::cfg *cfg);

  ::analysis::lattice_elem *eval(size_t node);
  std::queue<size_t> initial();

  ::analysis::lattice_elem *get(size_t node);
  void update(size_t node, lattice_elem *state);

  std::set<size_t> dependants(size_t node_id);
};

}  // namespace reaching_defs
}  // namespace analysis
