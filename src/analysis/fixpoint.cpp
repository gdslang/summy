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
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node_visitor.h>
#include <experimental/optional>

using std::experimental::optional;
using std::experimental::nullopt;

using namespace cfg;
using namespace std;
using namespace analysis;

analysis::fixpoint::fixpoint(class fp_analysis *analysis, jd_manager &jd_man, bool widening)
    : analysis(analysis), jd_man(jd_man), widening(widening), max_its(0) {}

void fixpoint::iterate() {
  updated.clear();
  node_iterations.clear();
  set<size_t> pending = analysis->pending();

  node_compare_t addr_comparer = [&](size_t const &a, size_t const &b) -> optional<bool> {
    size_t addr_a = jd_man.machine_address_of(a);
    size_t addr_b = jd_man.machine_address_of(b);
    if(addr_a < addr_b)
      return true;
    else if(addr_a > addr_b)
      return false;
    else
      return nullopt;
  };
  auto node_comparers =
    vector<node_compare_t>({analysis->get_fixpoint_node_comparer(), addr_comparer, node_compare_default});

  worklist = fp_priority_queue(node_comparers);
  analysis->clear_pending();
  fp_priority_queue postprocess_worklist(node_comparers);

  auto end = [&]() {
    //    cout << "wl: " << worklist.size() << endl;
    //    cout << "pp: " << postprocess_worklist.size() << endl;
    if(!pending.empty()) return false;
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
    if(!worklist.empty()) {
      return worklist.pop();
    } else {
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

    bool _continue = false;
    static optional<size_t> function_last;
    analysis_visitor av(true);
    av._([&](summary_dstack *sd) {
      //  cout << "PENDING: " << pending.size() << endl;
      auto nits_it = node_iterations.find(node_id);
      if(nits_it != node_iterations.end()) {
        nits_it->second++;
        if(nits_it->second > max_its || nits_it->second > 12) {
          cout << "Fixpoint -- New maximal iteration count: " << nits_it->second << endl;
          cout << "Fixpoint -- Average iteration count: " << avg_iteration_count() << endl;
          cout << "\tMachine address: 0x" << hex << jd_man.machine_address_of(node_id) << dec << endl;
          sd->print_callstack(node_id);
          max_its = nits_it->second;
          print_distribution_total();
//          cout << "node id: " << node_id << endl;
//          cout << "\tMachine address: 0x" << hex << jd_man.machine_address_of(node_id) << dec << endl;
//          cout << *analysis->get(node_id) << endl;
        }
//        if(nits_it->second > 20) _continue = true;
      } else
        node_iterations[node_id] = 1;
//      static size_t machine_address_last = 0;
//      size_t machine_address_current = jd_man.machine_address_of(node_id);
//      if(machine_address_current != machine_address_last) {
//        machine_address_last = machine_address_current;
//        cout << "\tMachine address: 0x" << hex << machine_address_current << dec << endl;
//      }
      //      optional<size_t> function_current = sd->get_lowest_function_address(node_id);
      //      if(function_current && (!function_last || function_last.value() != function_current.value())) {
      //        sd->print_callstack(node_id);
      //        function_last = function_current;
      //      }
    });
//    analysis->accept(av);
    if(_continue) continue;


    node_visitor nv;
    nv._([&](address_node *an) { machine_addresses.insert(an->get_address()); });
    analysis->get_cfg()->get_node_payload(node_id)->accept(nv);

    //    if(node_id == 11) cout << "NODE 11!!" << endl;
    //    machine_addresses.insert(jd_man.machine_address_of(node_id));
    //    if(machine_addresses.size() % 1000 == 0)
    //          cout << "Analyzed " << machine_addresses.size() << " machine addresses." << endl;
    // cout << *analysis->get(node_id) << endl;


    //    if(max_its > 2000)
    //      break;
    // Neue Maschinenadressen ausgeben für Fortschritt...?

    //    if(nits_it->second > 20) {
    //          cout << "Node: " << node_id << endl;

    //    }

    bool propagate;
    bool needs_postprocessing = false;
    shared_ptr<domain_state> accumulator;
    bool accumulator_set = false;
    auto &constraints = analysis->constraints_at(node_id);
    if(constraints.size() > 0) {
      //      shared_ptr<domain_state> current = analysis->get(node_id);
      //      current->check_consistency();
      bool backward = false;

      auto process_constraint = [&](size_t node_other, constraint_t constraint) {
        //        cout << "Constraint from " << node_other << " to " << node_id << endl;
        //                cout << *analysis->get(node_other) << endl;

        /*
         * Evaluate constraint
         */
        //        cout << "~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        auto evaluated = constraint();
        if(constraints.size() == 1)
          analysis->unref(node_other);
        else
          analysis->ref(node_other, nullopt);

        //        cout << "++++++++++++++++++++++++" << endl;

        //        cout << "Evaluated: " << *evaluated << endl;
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
        jump_dir jd = jd_man.jump_direction(node_other, node_id);

        backward = backward || jd != FORWARD;
        if(widening && jd == BACKWARD) {
          //          cout << "Current: " << *current << endl;
          //          cout << "Evaluated: " << *evaluated << endl;
          //                    cout << "Back jump from " << node_other << " to " << node_id << endl;
          domain_state *boxed;
          tie(boxed, needs_postprocessing) = analysis->get(node_id)->box(evaluated.get(), node_id);
          evaluated = shared_ptr<domain_state>(boxed);
          //          cout << "Boxed: " << *evaluated << endl;
        }

        //        cout << "============================" << endl;
        //        cout << "evaluated:" << endl
        //             << *evaluated << endl;

        if(accumulator_set) {
          //                    cout << "accumulator:" << endl
          //                         << *accumulator << endl;
          //          accumulator->check_consistency();
          //          evaluated->check_consistency();

          accumulator = shared_ptr<domain_state>(evaluated->join(accumulator.get(), node_id));
          //          cout << *accumulator << endl;

          //          accumulator->check_consistency();

        } else {
          accumulator = evaluated;
          accumulator_set = true;
        }

        //                cout << "accumulator (after):" << endl << *accumulator << endl;
      };

      analysis->record_updates();
      for(auto constraint_it = constraints.begin(); constraint_it != constraints.end(); constraint_it++)
        process_constraint(constraint_it->first, constraint_it->second);
      if(analysis->record_stop_commit()) {
        //        cout << "Comitted updates..." << endl;
        for(size_t node : analysis->pending()) {
          //                    cout << "====> Pushing node: " << node << endl;
          //    cout << this << endl;

          /*
           * Todo: Which one is better?
           */
          worklist.push(node);
          //          pending.insert(node);
        }
        analysis->clear_pending();
      }
      //      else
      //        cout << "Nothing to commit..." << endl;

      //      cout << "Current: " << *current << endl;
      //      cout << "Acc: " << *accumulator << endl;

      //      propagate = !(*current >= *accumulator);
      /*
       * No monotonicity because of the box operator
       */

      //      cout << "++++++ current:" << endl << *current << endl;
      //      cout << "++++++ acc:" << endl << *accumulator << endl;

      if(backward || constraints.size() != 1) analysis->ref(node_id, nullopt);

      propagate = (!backward && constraints.size() == 1) || !(*analysis->get(node_id) == *accumulator);

      //            cout << "prop: " << propagate << endl;
    } else
      /*
       * If the node has no incoming analysis dependency edges, we keep its default
       * state.
       */
      propagate = false;

    //     cout << "Propagate: " << propagate << endl;

    if(propagate) {
      if(needs_postprocessing) {
        //                cout << "====> Postproc: " << node_id << endl;
        postprocess_worklist.push(node_id);
      }
      //            cout << node_id << " current " << *analysis->get(node_id) << endl;
      //            cout << node_id << " XX->XX " << *accumulator << endl;
      //      accumulator->check_consistency();
      //            cout << "Updating..." << endl;
      analysis->update(node_id, accumulator);
      updated.insert(node_id);

      //      cout << "Updated " << node_id << endl;
      //      cout << *analysis->get(node_id) << endl;
    }

    //    cout << "Seen: " << (seen.find(node_id) != seen.end()) << endl;
    //    cout << "Number of deps: " << analysis->dependants(node_id).size() << endl;

    if(propagate || seen.find(node_id) == seen.end()) {
      auto dependants = analysis->dependants(node_id);
      for(auto dependant : dependants) {
        //                cout << "====>  Pushing " << dependant << " as dep. of " << node_id << endl;
        worklist.push(dependant);
      }
      //      cout << "Deps: " << dependants.size() << endl;
      //      cout << "Children: " << analysis->get_cfg()->out_edge_payloads(node_id)->size() << endl;
      analysis->ref(node_id, dependants.size());
    }
    auto dirty_nodes = analysis->dirty_nodes();
    for(auto dirty : dirty_nodes) {
      //            cout << "====> Adding dirty node: " << dirty << endl;
      worklist.push(dirty);
    }
    if(dirty_nodes.size() > 0) analysis->ref(node_id, dirty_nodes.size());

    seen.insert(node_id);

    //    cout << "END OF FP_ROUND" << endl;
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

size_t analysis::fixpoint::analyzed_addresses() {
  return machine_addresses.size();
}
