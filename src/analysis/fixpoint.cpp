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
  updated.clear();
  set<size_t> worklist = analysis->pending();

  while(!worklist.empty()) {
    size_t node_id;
    auto it = worklist.begin();
    node_id = *it;
    worklist.erase(it);

//    cout << "Next node: " << node_id << endl;

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

//      cout << "Current: " << *current << endl;
//      cout << "Evaluated: " << *evaluated << endl;

      propagate = !(*current >= *evaluated);
    } else
    /*
     * If the node has no incoming analysis dependency edges, we keep its default
     * state.
     */
    propagate = false;

//    cout << "Propagate: " << propagate << endl;

    if(propagate) {
      analysis->update(node_id, evaluated);
      updated.insert(node_id);
    }

    if(propagate || seen.find(node_id) == seen.end()) {
      auto dependants = analysis->dependants(node_id);
      for(auto dependant : dependants)
        worklist.insert(dependant);
    }

    seen.insert(node_id);
  }
}

void fixpoint::notify(const vector<::cfg::update> &updates) {
  analysis->update(updates);

  for(auto &update : updates) {
    seen.erase(update.from);
    seen.erase(update.to);
  }

  iterate();
}
