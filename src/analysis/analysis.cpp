/*
 * analysis.cpp
 *
 *  Created on: Oct 15, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/analysis.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/bfs_iterator.h>
#include <algorithm>
#include <vector>
#include <set>

using namespace analysis;
using namespace cfg;
using namespace std;

void ::analysis::analysis::init_fixpoint_pending() {
  for(size_t i = 0; i < cfg->node_count(); i++)
    fixpoint_pending.insert(i);

  for(auto &deps : _dependants)
    for(auto dep : deps.second) {
      fixpoint_pending.erase(dep);
    }
}

::analysis::analysis::analysis(class cfg *cfg) :
    cfg(cfg) {
}

void ::analysis::analysis::init() {
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edges(node_id);
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      add_constraint(node_id, edge_it->first, edge_it->second);
      add_dependency(node_id, edge_it->first);
    }
  }

  init_fixpoint_pending();
  init_state();
}

void ::analysis::analysis::update(vector<struct update> const &updates) {
  fixpoint_pending.clear();

  auto print_set = [](auto &s) {
    cout << "{";
    bool first = true;
    for(auto &es : s) {
      if(!first)
        cout << ", ";
      cout << es;
      first = false;
    }
    cout << "}" << endl;
  };

  set<size_t> fp_pend_new;
  for(auto &update : updates) {
    switch(update.kind) {
      case INSERT: {
        fp_pend_new.insert(update.from);
        fp_pend_new.insert(update.to);
        break;
      }
      case ERASE: {
        break;
      }
    }
  }

//  print_set(fp_pend_new);

//  set<size_t> fp_pend_rm;
  for(auto &update : updates) {
    switch(update.kind) {
      case INSERT: {
        cout << "INSERT " << update.from << " -> " << update.to << endl;
        if(cfg->contains_edge(update.from, update.to))
          add_constraint(update.from, update.to, cfg->out_edges(update.from)->at(update.to));
        add_dependency(update.from, update.to);
//        fp_pend_rm.insert(add_dependency(update.from, update.to));
        break;
      }
      case ERASE: {
        cout << "ERASE " << update.from << " -> " << update.to << endl;
        remove_constraint(update.from, update.to);
        remove_dependency(update.from, update.to);
//        fp_pend_rm.erase(remove_dependency(update.from, update.to));
        break;
      }
    }
  }

//  print_set(fp_pend_rm);
//
//  set_difference(fp_pend_new.begin(), fp_pend_new.end(), fp_pend_rm.begin(), fp_pend_rm.end(),
//      inserter(fixpoint_pending, fixpoint_pending.begin()));

  fixpoint_pending = fp_pend_new;
  for(auto fp_pend : fp_pend_new)
    for(auto &dep : _dependants[fp_pend])
      fixpoint_pending.erase(dep);

  print_set(fixpoint_pending);

  init_state();
}

std::ostream &::analysis::operator <<(std::ostream &out, analysis &_this) {
  _this.put(out);
  return out;
}
