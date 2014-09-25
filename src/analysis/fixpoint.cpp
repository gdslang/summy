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

void fixpoint::iterate() {
  queue<size_t> worklist = analysis->initial();
  while(!worklist.empty()) {
    size_t node_id = worklist.front();
    worklist.pop();

    lattice_elem *evaluated = analysis->eval(node_id);
    lattice_elem *current = analysis->get(node_id);
    if(evaluated > current) {
      lattice_elem *lubbed = current->lub(evaluated);
      analysis->update(node_id, lubbed);

      auto dependants = analysis->dependants(node_id);
      for(auto dependant : dependants)
        worklist.push(dependant);
    }

  }
}
