/*
 * analysis.cpp
 *
 *  Created on: Oct 15, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/analysis.h>

using namespace analysis;

void ::analysis::analysis::init_fixpoint_initial() {
  for(size_t i = 0; i < cfg->node_count(); i++)
    fixpoint_initial.insert(i);

  for(auto deps : _dependants)
    for(auto dep : deps) {
      fixpoint_initial.erase(dep);
    }
}

::analysis::analysis::analysis(cfg::cfg* cfg) :
    cfg(cfg), constraints(cfg->node_count()), _dependants(cfg->node_count()) {
}

void ::analysis::analysis::init() {
  init_constraints();
  init_dependants();
  init_fixpoint_initial();
}

std::ostream &::analysis::operator <<(std::ostream &out, analysis &_this) {
  _this.put(out);
  return out;
}
