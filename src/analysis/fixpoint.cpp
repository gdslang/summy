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

using namespace std;
using namespace analysis;

#include <iostream>

void fixpoint::iterate() {
  set<size_t> worklist = analysis->initial();
  set<size_t> seen;
  while(!worklist.empty()) {
//    size_t node_id = worklist.front();
//    worklist.pop();
    size_t node_id;
    auto it = worklist.begin();
    node_id = *it;
    worklist.erase(it);

    shared_ptr<lattice_elem> evaluated = analysis->eval(node_id);
    shared_ptr<lattice_elem> current = analysis->get(node_id);

    bool propagate = !(*current >= *evaluated);

    if(propagate) {
//      shared_ptr<lattice_elem> lubbed = shared_ptr<lattice_elem>(current->lub(evaluated.get()));
      analysis->update(node_id, evaluated);
    }

//    if(seen.find(node_id) == seen.end())
    if(propagate || seen.find(node_id) == seen.end()) {
      auto dependants = analysis->dependants(node_id);
      for(auto dependant : dependants)
        worklist.insert(dependant);
    }

    seen.insert(node_id);
  }
}
