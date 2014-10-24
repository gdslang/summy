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
  set<size_t> worklist = analysis->pending();

  /*
   * The initial worklist contains newly added start nodes for the iteration,
   * we should not consider them as seen...
   *
   * Problem: This is not enough, what about new edges between existing nodes?
   */
  for(auto item : worklist)
    seen.erase(item);

  while(!worklist.empty()) {
    size_t node_id;
    auto it = worklist.begin();
    node_id = *it;
    worklist.erase(it);

    cout << "Next node: " << node_id << endl;

    bool propagate;
    shared_ptr<lattice_elem> evaluated;
    auto &constraints = analysis->constraints_at(node_id);
    if(constraints.size() > 0) {
      auto constraint_it = constraints.begin();
      evaluated = constraint_it->second();
      for(constraint_it++; constraint_it != constraints.end(); constraint_it++) {
        auto calc = constraint_it->second();
        evaluated = shared_ptr<lattice_elem>(calc->lub(evaluated.get(), node_id));
      }
      shared_ptr<lattice_elem> current = analysis->get(node_id);

      cout << "Current: " << *current << endl;
      cout << "Evaluated: " << *evaluated << endl;

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
