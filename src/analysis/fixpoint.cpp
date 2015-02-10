/*
 * fixpoint.cpp
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/fixpoint.h>
#include <summy/analysis/domain_state.h>
#include <summy/analysis/fp_analysis.h>
#include <queue>
#include <iostream>

using namespace std;
using namespace analysis;

void analysis::fp_priority_queue::push(size_t value) {
  inner.insert(value);
}

size_t analysis::fp_priority_queue::pop() {
  size_t value;
  auto it = inner.begin();
  value = *it;
  inner.erase(it);
  return value;
}

bool analysis::fp_priority_queue::empty() {
  return inner.empty();
}

void fixpoint::iterate() {
  updated.clear();
  fp_priority_queue worklist = analysis->pending();

  while(!worklist.empty()) {
    size_t node_id = worklist.pop();

//    cout << "Next node: " << node_id << endl;

    bool propagate;
    shared_ptr<domain_state> evaluated;
    auto &constraints = analysis->constraints_at(node_id);
    if(constraints.size() > 0) {
      auto constraint_it = constraints.begin();
      evaluated = constraint_it->second();
      for(constraint_it++; constraint_it != constraints.end(); constraint_it++) {
        auto calc = constraint_it->second();
        evaluated = shared_ptr<domain_state>(calc->join(evaluated.get(), node_id));
      }
      shared_ptr<domain_state> current = analysis->get(node_id);

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
        worklist.push(dependant);
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
