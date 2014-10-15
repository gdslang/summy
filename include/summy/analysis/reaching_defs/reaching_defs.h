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
  std::vector<std::set<size_t>> _dependants;
  std::set<size_t> fixpoint_initial;

  virtual void init_constraints();
  virtual void init_dependants();
public:
  reaching_defs(cfg::cfg *cfg);
  ~reaching_defs();

  std::shared_ptr<lattice_elem> bottom();
  std::shared_ptr<lattice_elem> start_value();

  std::shared_ptr<lattice_elem> get(size_t node);
  void update(size_t node, std::shared_ptr<lattice_elem> state);

  void put(std::ostream &out);
};

}  // namespace reaching_defs
}  // namespace analysis
