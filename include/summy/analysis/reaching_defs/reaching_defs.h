/*
 * reaching_def.h
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/analysis.h>
#include <summy/analysis/reaching_defs/rd_elem.h>
#include <vector>
#include <functional>
#include <set>
#include <ostream>
#include <cppgdsl/rreil/id/id.h>
#include <memory>

namespace analysis {
namespace reaching_defs {

class reaching_defs : public analysis {
public:
  typedef std::vector<std::shared_ptr<rd_elem>> state_t;
private:
  state_t state;
  std::vector<std::function<std::shared_ptr<rd_elem>()>> constraints;
  std::vector<std::set<size_t>> _dependants;

  void init_constraints();
  void init_dependants();
public:
  reaching_defs(cfg::cfg *cfg);
  ~reaching_defs();

  shared_ptr<lattice_elem> bottom();

  shared_ptr<lattice_elem> eval(size_t node);
  std::set<size_t> initial();

  shared_ptr<lattice_elem> get(size_t node);
  void update(size_t node, shared_ptr<lattice_elem> state);

  std::set<size_t> dependants(size_t node_id);

  friend std::ostream &operator<< (std::ostream &out, reaching_defs &_this);
};

std::ostream &operator<<(std::ostream &out, reaching_defs &_this);

}  // namespace reaching_defs
}  // namespace analysis
