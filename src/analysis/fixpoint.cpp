/*
 * fixpoint.cpp
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/fixpoint.h>
#include <summy/analysis/domain_state.h>
#include <summy/analysis/fp_analysis.h>
#include <summy/analysis/global_analysis/global_state.h>
#include <summy/cfg/jd_manager.h>
#include <queue>
#include <iostream>

using namespace cfg;
using namespace std;
using namespace analysis;


void fixpoint::iterate() {
  updated.clear();
  node_iterations.clear();
  fp_priority_queue worklist = fp_priority_queue(analysis->pending(), analysis->get_fixpoint_node_comparer());
  fp_priority_queue postprocess_worklist(analysis->get_fixpoint_node_comparer());

  auto end = [&]() {
    if(worklist.empty()) {
      if(postprocess_worklist.empty())
        return true;
      else {
        worklist = postprocess_worklist;
        postprocess_worklist.clear();
        return false;
      }
    } else
      return false;
  };

  while(!end()) {
    size_t node_id = worklist.pop();

    auto nits_it = node_iterations.find(node_id);
    if(nits_it != node_iterations.end())
      nits_it->second++;
    else
      node_iterations[node_id] = 0;

//    cout << "Next node: " << node_id << endl;

    bool propagate;
    shared_ptr<domain_state> accumulator;
    bool accumulator_set = false;
    auto &constraints = analysis->constraints_at(node_id);
    if(constraints.size() > 0) {
      shared_ptr<domain_state> current = analysis->get(node_id);
      auto process_constraint = [&](size_t node_other, constraint_t constraint) {
        //        cout << "Constraint from " << node_other << " to " << node_id << endl;

        /*
         * Evaluate constraint
         */
        //        cout << "~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        auto evaluated = constraint();
        //        cout << "++++++++++++++++++++++++" << endl;

        //                                cout << "Evaluated: " << *evaluated << endl;

        /*
         * Apply box operator if this edge is a 'back edge' with respect
         * to the given ordering => too many strange widening points
         *
         * Now: Use jd_manager to find edges to smaller addresses
         */
        /*
         * Todo: Backward analysis?
         */
        //        cout << "Current: " << *current << endl;
        if(jd_man.jump_direction(node_other, node_id) == BACKWARD) {
//          cout << "Back jump from " << node_other << " to " << node_id << endl;
          domain_state *boxed;
          bool needs_postprocessing;
          tie(boxed, needs_postprocessing) = current->box(evaluated.get(), node_id);
          evaluated = shared_ptr<domain_state>(boxed);
//          cout << "Result: " << endl << *evaluated << endl;
          if(needs_postprocessing) {
//            cout << "Postproc: " << node_id << endl;
            postprocess_worklist.push(node_id);
          }
//                    cout << "Boxed: " << *evaluated << endl;
        }

        //        cout << "============================" << endl;
        //                cout << "evaluated:" << endl << *evaluated << endl;

        if(accumulator_set) {
          //          cout << "accumulator:" << endl << *accumulator << endl;

          accumulator = shared_ptr<domain_state>(evaluated->join(accumulator.get(), node_id));
        } else {
          accumulator = evaluated;
          accumulator_set = true;
        }

        //        cout << "accumulator (after):" << endl << *accumulator << endl;
      };

      analysis->record_updates();
      for(auto constraint_it = constraints.begin(); constraint_it != constraints.end(); constraint_it++)
        process_constraint(constraint_it->first, constraint_it->second);
      analysis->record_stop_commit();

      //      cout << "Current: " << *current << endl;
      //      cout << "Acc: " << *accumulator << endl;

      //      propagate = !(*current >= *accumulator);
      /*
       * No monotonicity because of the box operator
       */

      //      cout << "++++++ current:" << endl << *current << endl;
      //      cout << "++++++ acc:" << endl << *accumulator << endl;

      propagate = !(*current == *accumulator);

      //      cout << "prop: " << propagate << endl;
    } else
      /*
       * If the node has no incoming analysis dependency edges, we keep its default
       * state.
       */
      propagate = false;


    //    cout << "Propagate: " << propagate << endl;

    if(propagate) {
      analysis->update(node_id, accumulator);
      updated.insert(node_id);
    }

    if(propagate || seen.find(node_id) == seen.end()) {
      auto dependants = analysis->dependants(node_id);
      for(auto dependant : dependants) {
//        cout << "Pushing " << dependant << endl;
        worklist.push(dependant);
      }
    }
    auto dirty_nodes = analysis->dirty_nodes();
    for(auto dirty : dirty_nodes) {
//      cout << "Adding dirty node: " << dirty << endl;
      worklist.push(dirty);
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

size_t analysis::fixpoint::max_iter() {
  size_t max = 0;
  for(auto nits_it : node_iterations)
    if(nits_it.second > max)
      max = nits_it.second;
  return max;
}
