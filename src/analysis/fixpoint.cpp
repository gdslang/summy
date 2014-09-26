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

#include <summy/analysis/reaching_defs/lattice_elem.h>
#include <iostream>
void fixpoint::iterate() {
  set<size_t> worklist = analysis->initial();
  while(!worklist.empty()) {
//    size_t node_id = worklist.front();
//    worklist.pop();
    size_t node_id;
    auto it = worklist.begin();
    node_id = *it;
    worklist.erase(it);

    lattice_elem *evaluated = analysis->eval(node_id);

    lattice_elem *current = analysis->get(node_id);
//    cout << "+++++---" << endl;
//    cout << "evaluated: " << *(dynamic_cast<::analysis::reaching_defs::lattice_elem*>(evaluated)) << endl;
//    cout << "current: " << *(dynamic_cast<::analysis::reaching_defs::lattice_elem*>(current)) << endl;
    if(*evaluated > *current) {

      lattice_elem *lubbed = current->lub(evaluated);
//      cout << "lubbed: " << *(dynamic_cast<::analysis::reaching_defs::lattice_elem*>(lubbed)) << endl;

//      cout << "Updating " << node_id << "..." << endl;
      analysis->update(node_id, lubbed);

      auto dependants = analysis->dependants(node_id);
      for(auto dependant : dependants) {
//        cout << "Inserting " << dependant << "..." << endl;
        worklist.insert(dependant);
      }
    }

    delete evaluated;
  }
}
