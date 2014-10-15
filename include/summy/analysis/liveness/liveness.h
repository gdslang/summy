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

  virtual void init_constraints();
  virtual void init_dependants();
public:
  liveness(cfg::cfg *cfg);
  ~liveness();

  shared_ptr<lattice_elem> bottom();

  shared_ptr<lattice_elem> get(size_t node);
  void update(size_t node, shared_ptr<lattice_elem> state);

  void put(std::ostream &out);
};

}  // namespace reaching_defs
}  // namespace analysis
