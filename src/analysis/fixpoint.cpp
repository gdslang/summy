/*
 * fixpoint.cpp
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/fixpoint.h>
#include <summy/analysis/analysis.h>
#include <summy/analysis/lattice_elem.h>
#include <queue>
#include <iostream>

using namespace std;
using namespace analysis;

void fixpoint::iterate() {
  set<size_t> worklist = analysis->initial();
  set<size_t> seen;
  while(!worklist.empty()) {
    size_t node_id;
    auto it = worklist.begin();
    node_id = *it;
    worklist.erase(it);

    bool propagate;
    shared_ptr<lattice_elem> evaluated;
    auto constraints = analysis->constraints_at(node_id);
    if(constraints.size() > 0) {
      evaluated = constraints[0]();
      for(size_t i = 1; i < constraints.size(); i++) {
        auto calc = constraints[i]();
        evaluated = shared_ptr<lattice_elem>(calc->lub(evaluated.get(), node_id));
      }
      shared_ptr<lattice_elem> current = analysis->get(node_id);
      propagate = !(*current >= *evaluated);
    } else
    /*
     * If the node has no incoming analysis dependency edges, we keep its default
     * state.
     */
    propagate = false;

    if(propagate) {
      analysis->update(node_id, evaluated);
    }

    if(propagate || seen.find(node_id) == seen.end()) {
      auto dependants = analysis->dependants(node_id);
      for(auto dependant : dependants)
        worklist.insert(dependant);
    }

    seen.insert(node_id);
  }
}
