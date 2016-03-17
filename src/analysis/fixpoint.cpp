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
#include <assert.h>

using namespace cfg;
using namespace std;
using namespace analysis;

analysis::fixpoint::fixpoint(class fp_analysis *analysis, jd_manager &jd_man, bool widening)
    : analysis(analysis), jd_man(jd_man), widening(widening), max_its(0) {}

void fixpoint::iterate() {
  updated.clear();
  node_iterations.clear();
  set<size_t> pending = analysis->pending();
  worklist = fp_priority_queue(analysis->get_fixpoint_node_comparer());
  analysis->clear_pending();
  fp_priority_queue postprocess_worklist(analysis->get_fixpoint_node_comparer());

  auto end = [&]() {
//    cout << "wl: " << worklist.size() << endl;
//    cout << "pp: " << postprocess_worklist.size() << endl;
    if(!pending.empty())
      return false;
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

  auto next = [&]() {
    if(!worklist.empty())
        return worklist.pop();
    else {
      auto it = pending.begin();
      assert(it != pending.end());
      size_t next_element = *it;
      pending.erase(it);
      return next_element;
    }
  };

  while(!end()) {
    size_t node_id = next();

//    cout << "Next node: " << node_id << endl;
    //    if(node_id == 11) cout << "NODE 11!!" << endl;
    //    cout << "\tMachine address: 0x" << hex << jd_man.machine_address_of(node_id) << dec << endl;
    //    if(node_id == 26) cout << *analysis->get(node_id) << endl;

    auto nits_it = node_iterations.find(node_id);
    if(nits_it != node_iterations.end()) {
      nits_it->second++;
      if(nits_it->second > max_its) {
        cout << "Fixpoint -- New maximal iteration count: " << nits_it->second << endl;
        cout << "Fixpoint -- Average iteration count: " << avg_iteration_count() << endl;
        max_its = nits_it->second;
        print_distribution_total();
      }
    } else
      node_iterations[node_id] = 1;

//    if(max_its > 2000)
//      break;
    // Neue Maschinenadressen ausgeben für Fortschritt...?

//    if(nits_it->second > 20) {
//      cout << "Node: " << node_id << endl;
//      static size_t machine_address_last = 0;
//      size_t machine_address_current = jd_man.machine_address_of(node_id);
//      if(machine_address_current != machine_address_last) {
//        machine_address_last = machine_address_current;
//        cout << "\tMachine address: 0x" << hex << machine_address_current << dec << endl;
//      }
//    }



    bool propagate;
    bool needs_postprocessing = false;
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

        //                cout << "Evaluated: " << *evaluated << endl;
        //                if(node_id == 67) cout << "Evaluated: " << *evaluated << endl;

        /*
         * Apply box operator if this edge is a 'back edge' with respect
         * to the given ordering => too many strange widening points
         *
         * Now: Use jd_manager to find edges to smaller addresses
         */
        /*
         * Todo: Backward analysis?
         */
        if(widening && jd_man.jump_direction(node_other, node_id) == BACKWARD) {
          //          cout << "Current: " << *current << endl;
          //          cout << "Evaluated: " << *evaluated << endl;
          //          cout << "Back jump from " << node_other << " to " << node_id << endl;
          domain_state *boxed;
          tie(boxed, needs_postprocessing) = current->box(evaluated.get(), node_id);
          evaluated = shared_ptr<domain_state>(boxed);
          //          cout << "Boxed: " << *evaluated << endl;
        }

        //        cout << "============================" << endl;
        //        cout << "evaluated:" << endl
        //             << *evaluated << endl;

        if(accumulator_set) {
          //                    cout << "accumulator:" << endl
          //                         << *accumulator << endl;

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
      if(analysis->record_stop_commit()) {
        for(size_t node : analysis->pending()) {
//          cout << "====> Pushing node: " << node << endl;
          //    cout << this << endl;
          //worklist.push(node);
          pending.insert(node);
        }
        analysis->clear_pending();
      }

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

    //     cout << "Propagate: " << propagate << endl;

    if(propagate) {
      if(needs_postprocessing) {
//        cout << "====> Postproc: " << node_id << endl;
        postprocess_worklist.push(node_id);
      }
      //            cout << node_id << " current " << *analysis->get(node_id) << endl;
      //            cout << node_id << " XX->XX " << *accumulator << endl;
      //      accumulator->check_consistency();
      //      cout << "FOOOO" << endl;
      analysis->update(node_id, accumulator);
      updated.insert(node_id);
    }

    //    cout << "Seen: " << (seen.find(node_id) != seen.end()) << endl;

    if(propagate || seen.find(node_id) == seen.end()) {
      auto dependants = analysis->dependants(node_id);
      for(auto dependant : dependants) {
//        cout << "====>  Pushing " << dependant << " as dep. of " << node_id << endl;
        worklist.push(dependant);
      }
    }
    auto dirty_nodes = analysis->dirty_nodes();
    for(auto dirty : dirty_nodes) {
//      cout << "====> Adding dirty node: " << dirty << endl;
      worklist.push(dirty);
    }

    seen.insert(node_id);
  }
}

void fixpoint::notify(const vector<::cfg::update> &updates) {
  //    analysis->update(updates);

  for(auto &update : updates) {
    seen.erase(update.from);
    seen.erase(update.to);
  }

  //  iterate();
}

size_t analysis::fixpoint::max_iter() {
  size_t max = 0;
  for(auto nits_it : node_iterations) {
    if(nits_it.second > max) max = nits_it.second;
  }

  return max;
}

void analysis::fixpoint::print_distribution() {
  map<size_t, set<size_t>> back;

  for(auto nits_it : node_iterations)
    back[nits_it.second].insert(nits_it.first);

  for(auto it : back) {
    cout << it.first << " its: ";
    for(auto it2 : it.second)
      cout << it2 << ", ";
    cout << endl;
  }
}

void analysis::fixpoint::print_distribution_total() {
  map<size_t, size_t> back;

  for(auto nits_it : node_iterations) {
    auto back_it = back.find(nits_it.second);
    if(back_it != back.end())
      back_it->second += 1;
    else
      back[nits_it.second] = 1;
  }

  for(auto it : back) {
    cout << it.first << " its: ";
    cout << it.second;
    cout << endl;
  }
}

double analysis::fixpoint::avg_iteration_count() {
  double sum = 0;
  for(auto nits_it : node_iterations)
    sum += nits_it.second;
  return sum / node_iterations.size();
}
