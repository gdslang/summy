/*
 * liveness.h
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/analysis.h>
#include <summy/analysis/lattice_elem.h>
#include <summy/analysis/liveness/lv_elem.h>
#include <memory>
#include <vector>

namespace analysis {
namespace liveness {

class liveness : public analysis {
public:
  typedef std::vector<std::shared_ptr<lv_elem>> state_t;
  typedef std::function<std::shared_ptr<lv_elem>()> constraint_t;
private:
  state_t state;
  std::vector<constraint_t> constraints;
  std::vector<std::set<size_t>> _dependants;

  void init_constraints();
  void init_dependants();
public:
  liveness(cfg::cfg *cfg);
  ~liveness();

  shared_ptr<lattice_elem> bottom();

  shared_ptr<lattice_elem> eval(size_t node);
  std::set<size_t> initial();

  shared_ptr<lattice_elem> get(size_t node);
  void update(size_t node, shared_ptr<lattice_elem> state);

  std::set<size_t> dependants(size_t node_id);

  friend std::ostream &operator<< (std::ostream &out, liveness &_this);
};

std::ostream &operator<<(std::ostream &out, liveness &_this);

}  // namespace reaching_defs
}  // namespace analysis
