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
#include <ostream>
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

  lattice_elem *bottom();

  lattice_elem *eval(size_t node);
  std::queue<size_t> initial();

  lattice_elem *get(size_t node);
  void update(size_t node, ::analysis::lattice_elem *state);

  std::set<size_t> dependants(size_t node_id);

  friend std::ostream &operator<< (std::ostream &out, reaching_defs &_this);
};

std::ostream &operator<<(std::ostream &out, reaching_defs &_this);

}  // namespace reaching_defs
}  // namespace analysis
