/*
 * fixpoint.cpp
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <experimental/optional>
#include <iostream>
#include <queue>
#include <summy/analysis/domain_state.h>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/analysis/fixpoint.h>
#include <summy/analysis/fp_analysis.h>
#include <summy/analysis/global_analysis/global_state.h>
#include <summy/cfg/jd_manager.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node_visitor.h>

using std::experimental::optional;
using std::experimental::nullopt;

using namespace cfg;
using namespace std;
using namespace analysis;

analysis::fixpoint::fixpoint(
  class fp_analysis *analysis, jd_manager &jd_man, bool ref_management, bool widening)
    : analysis(analysis), jd_man(jd_man), ref_management(ref_management), widening(widening),
      max_its(0), construct_time(std::time(nullptr)) {}

void fixpoint::iterate() {
  updated.clear();
  node_iterations.clear();
  set<analysis_node> pending = analysis->pending();

  node_compare_t addr_comparer = [&](
    analysis_node const &a, analysis_node const &b) -> optional<bool> {
    size_t addr_a = jd_man.machine_address_of(a.id);
    size_t addr_b = jd_man.machine_address_of(b.id);
    if(addr_a < addr_b)
      return true;
    else if(addr_a > addr_b)
      return false;
    else
      return nullopt;
  };
  auto node_comparers = vector<node_compare_t>(
    {analysis->get_fixpoint_node_comparer(), addr_comparer, node_compare_default});

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
      auto next_element = *it;
      pending.erase(it);
      return next_element;
    }
  };

  std::time_t start_time = std::time(nullptr);
  std::time_t last_check = start_time;

  auto enqueue_updated_dirty = [&]() {
    for(auto ud_id : updated_dirty) {
      //       worklist.push(analysis_node(ud_id, 0));
      auto ctx_mappings = analysis->get_ctxful(ud_id);
      for(auto ctx_mapping : ctx_mappings) {
        size_t context = ctx_mapping.first;
        worklist.push(analysis_node(ud_id, context));
      }
    }
    updated_dirty.clear();
  };

  enqueue_updated_dirty();

  while(!end()) {
    analysis_node node = next();

    //     std::time_t current_time = std::time(nullptr);
    //     if(current_time - construct_time > 20 * 60) {
    //       cout << "\033[1;31m!!! TIME IS UP !!!\033[0m" << endl;
    //       break;
    //     }

    //    cout << "\033[1;31mNext iteration\033[0m" << endl;

    bool _continue = false;
    static optional<size_t> function_last;
    analysis_visitor av(true);
    bool is_sd = false;
    av._([&](summary_dstack *sd) {
      is_sd = true;
      //  cout << "PENDING: " << pending.size() << endl;
      auto nits_it = node_iterations.find(node.id);
      if(nits_it != node_iterations.end()) {
        nits_it->second++;
        // if(nits_it->second > max_its || nits_it->second > 12) {

        std::time_t current_time = std::time(nullptr);
        if(current_time - last_check > 10) {
          last_check = current_time;
          cout << "Fixpoint -- New maximal iteration count: " << nits_it->second << endl;
          cout << "Fixpoint -- Average iteration count: " << avg_iteration_count() << endl;
          cout << "\tMachine address: 0x" << hex << jd_man.machine_address_of(node.id) << dec
               << endl;
          sd->print_callstack(node.id);
          max_its = nits_it->second;
          print_distribution_total();
          //                    cout << "node id: " << node.id << endl;
          //                    cout << "\tMachine address: 0x" << hex <<
          //                    jd_man.machine_address_of(node.id)
          //                    << dec << endl;
          // cout << *analysis->get(node.id) << endl;
        }
        //        if(nits_it->second > 20) _continue = true;
      } else
        node_iterations[node.id] = 1;
      //      static size_t machine_address_last = 0;
      //      size_t machine_address_current = jd_man.machine_address_of(node_id);
      //      if(machine_address_current != machine_address_last) {
      //        machine_address_last = machine_address_current;
      //        cout << "\tMachine address: 0x" << hex << machine_address_current << dec << endl;
      //      }
      //      optional<size_t> function_current = sd->get_lowest_function_address(node_id);
      //      if(function_current && (!function_last || function_last.value() !=
      //      function_current.value())) {
      //        sd->print_callstack(node_id);
      //        function_last = function_current;
      //      }
    });
    /*
     * If this is commented out, the tests won't work ;-)
     */
    analysis->accept(av);
    if(_continue) continue;

//     if(is_sd) cout << "Next node: " << node << endl;
    
    //         static int ctr = 0;
    //         if(node.id == 32 || node.id == 46) {
    //           if(ctr > 5)
    //             continue;
    //           ctr++;
    //         }

    //    if(jd_man.machine_address_of(node_id) > 0x40190b)
    //      break;

    node_visitor nv;
    nv._([&](address_node *an) { machine_addresses.insert(an->get_address()); });
    analysis->get_cfg()->get_node_payload(node.id)->accept(nv);

    //    if(node_id == 11) cout << "NODE 11!!" << endl;
    //    machine_addresses.insert(jd_man.machine_address_of(node_id));
    //    if(machine_addresses.size() % 1000 == 0)
    //          cout << "Analyzed " << machine_addresses.size() << " machine addresses." << endl;
    //        cout << *analysis->get(node_id) << endl;


    //     if(max_its > 20) break;
    // Neue Maschinenadressen ausgeben für Fortschritt...?

    //    if(nits_it->second > 20) {
    //          cout << "Node: " << node_id << endl;

    //    }

    std::map<size_t, bool> propagate;
    bool needs_postprocessing = false;
    std::map<size_t, shared_ptr<domain_state>> accumulator;
    auto constraints = analysis->constraints_at(node.id);

    if(constraints.size() > 0) {
      //      shared_ptr<domain_state> current = analysis->get(node_id);
      //      current->check_consistency();
      bool backward = false;

      auto process_constraint = [&](size_t node_other, constraint_t constraint) {
        //                cout << *analysis->get(node_other) << endl;
        /*
         * Evaluate constraint
         */
        //        cout << "~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        auto evaluated_ctx = constraint(node.context);
        if(ref_management) {
          if(constraints.size() == 1)
            analysis->unref(node_other);
          else
            analysis->ref(node_other, nullopt);
        }

        //        cout << "++++++++++++++++++++++++" << endl;

        //        if(jd_man.machine_address_of(node_id) == 0x401908) cout << "Evaluated: " <<
        //         if(node_id == 303) {
        //           cout << "Constraint from " << node_other << " to " << node_id << endl;
        //           cout << *evaluated << endl;
        //         }
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

        jump_dir jd;
        if(node_other == node.id)
          jd = FORWARD;
        else
          jd = jd_man.jump_direction(node_other, node.id);

        for(auto &ev_it : evaluated_ctx) {
          size_t context = ev_it.first;
          auto evaluated = ev_it.second;

          backward = backward || jd != FORWARD;
          if(widening && jd == BACKWARD) {
            //          cout << "Current: " << *current << endl;
            //          cout << "Evaluated: " << *evaluated << endl;
            //                    cout << "Back jump from " << node_other << " to " << node_id <<
            //                    endl;
            domain_state *boxed;
            bool np;
            tie(boxed, np) =
              analysis->get_ctxful(node.id).at(context)->box(evaluated.get(), node.id);
            needs_postprocessing = np || needs_postprocessing;
            evaluated = shared_ptr<domain_state>(boxed);
            //          cout << "Boxed: " << *evaluated << endl;
          }

          //        cout << "============================" << endl;
          //                cout << "evaluated:" << endl
          //                     << *evaluated << endl;

          auto acc_it = accumulator.find(context);
          if(acc_it != accumulator.end()) {

            acc_it->second =
              shared_ptr<domain_state>(evaluated->join(acc_it->second.get(), node.id));

          } else
            accumulator[context] = evaluated;
        }
      };

      analysis->record_updates();
      for(auto constraint_it = constraints.begin(); constraint_it != constraints.end();
          constraint_it++)
        process_constraint(constraint_it->first, constraint_it->second);
      if(analysis->record_stop_commit()) {
        //        cout << "Comitted updates..." << endl;
        for(analysis_node node : analysis->pending()) {

          /*
           * Todo: Which one is better?
           */
          worklist.push(node);
          //          pending.insert(node);
        }
        analysis->clear_pending();
      }

      /*
       * No monotonicity because of the box operator
       */

      //      cout << "++++++ current:" << endl << *current << endl;
      //      cout << "++++++ acc:" << endl << *accumulator << endl;

      if(ref_management)
        if(backward || constraints.size() != 1) analysis->ref(node.id, nullopt);

      /**
       * Todo: This propagates to all contexts if one is unstable; should be changed
       * so that less updates occur
       */
      for(auto &acc_it : accumulator) {
        //         assert(propagate == false);
        auto current_state = analysis->get_ctxful(node.id);
        auto current_state_it = current_state.find(acc_it.first);

        auto acc = acc_it.second;
        size_t context = acc_it.first;

        propagate[context] = propagate[context] || (!backward && constraints.size() == 1) ||
                             current_state_it == current_state.end() ||
                             !(*current_state_it->second == *acc);

//         cout << propagate[context] << endl;
      }

      //            cout << "prop: " << propagate << endl;
    } // else
      /*
       * If the node has no incoming analysis dependency edges, we keep its default
       * state.
       */
    // propagate = false;

    //     cout << "Propagate: " << propagate << endl;


    //            cout << node_id << " current " << *analysis->get(node_id) << endl;
    //            cout << node_id << " XX->XX " << *accumulator << endl;
    //      accumulator->check_consistency();
    //            cout << "Updating..." << endl;

    for(auto &acc_it : accumulator) {
      auto acc = acc_it.second;
      size_t context_acc = acc_it.first;

      if(propagate[context_acc]) {
        if(context_acc == node.context && needs_postprocessing) {
          postprocess_worklist.push(node);
        }
        
        analysis->update(analysis_node(node.id, context_acc), acc);
        updated.insert(node.id);
      }

      //      cout << "Updated " << node_id << endl;
      //      cout << *analysis->get(node_id) << endl;
    }

    //    cout << "Seen: " << (seen.find(node_id) != seen.end()) << endl;
    //    cout << "Number of deps: " << analysis->dependants(node_id).size() << endl;

    //     if(!node_seen && node.id == 24)
    //       for(auto other : seen) {
    //         cout << "\t\t" << other << endl;
    //       }

    auto process_dependencies = [&](depdant_desc &dependants) {
      size_t pushes = 0;
      for(auto dependant : dependants.context_free_deps) {
        //                cout << "====>  Pushing " << dependant << " as dep. of " << node_id <<
        //                endl;
        for(auto acc_it : accumulator) {
          if(propagate[acc_it.first]) {
            worklist.push(analysis_node(dependant, acc_it.first));
            pushes++;
          }
        }
        //         if(!node_seen && accumulator.find(0) == accumulator.end()) {
        //           worklist.push(analysis_node(dependant, 0));
        //           pushes++;
        //         }
      }
      for(auto context_dep : dependants.context_deps) {
        for(auto depdant : context_dep.second) {
          worklist.push(analysis_node(depdant, context_dep.first));
          pushes++;
        }
      }
      //      cout << "Deps: " << dependants.size() << endl;
      //      cout << "Children: " << analysis->get_cfg()->out_edge_payloads(node_id)->size() <<
      //      endl;
      if(ref_management) {
        analysis->ref(node.id, pushes);
      }
    };

    auto dependants = analysis->dependants(node.id);
    process_dependencies(dependants);
    auto dirty_nodes = analysis->dirty_nodes();
    process_dependencies(dirty_nodes);

    //     cout << "Inserting " << node << endl;
    //     seen.insert(node);
    //    cout << "END OF FP_ROUND" << endl;
    enqueue_updated_dirty();
  }
}

void fixpoint::notify(const vector<::cfg::update> &updates) {
  //    analysis->update(updates);

  for(auto &update : updates) {
    //     cout << "Update from " << update.from << " to " << update.to << endl;

    updated_dirty.insert(update.from);
    updated_dirty.insert(update.to);

    //     seen.erase(update.from);
    //     seen.erase(update.to);
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

void analysis::fixpoint::print_hot_addresses() {
  map<size_t, set<size_t>> hot;

  for(auto it : node_iterations) {
    if(it.second < 20) continue;
    auto cfg = analysis->get_cfg();
    auto payload = cfg->get_node_payload(it.first);
    optional<size_t> addr = nullopt;
    node_visitor nv;
    nv._([&](address_node *an) { addr = an->get_address(); });
    payload->accept(nv);
    if(addr) hot[it.second].insert(*addr);
  }

  for(auto it : hot) {
    cout << it.first << " its:" << endl;
    for(auto addr : it.second)
      cout << "  Address: 0x" << hex << addr << dec << endl;
  }
}
