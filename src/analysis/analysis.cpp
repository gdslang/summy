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
      auto dep = gen_dependency(node_id, edge_it->first);
      _dependants[dep.source].insert(dep.sink);
    }
  }

  init_fixpoint_pending();
  init_state();
}

void ::analysis::analysis::update(vector<struct update> const &updates) {
  fixpoint_pending.clear();

//  auto print_set = [](auto &s) {
//    cout << "{";
//    bool first = true;
//    for(auto &es : s) {
//      if(!first)
//        cout << ", ";
//      cout << es;
//      first = false;
//    }
//    cout << "}" << endl;
//  };

  for(auto &update : updates) {
    switch(update.kind) {
      case INSERT: {
        fixpoint_pending.insert(update.from);
        fixpoint_pending.insert(update.to);
        break;
      }
      case ERASE: {
        break;
      }
    }
  }

  dependants_t local_deps;

  for(auto &update : updates) {
    auto dep = gen_dependency(update.from, update.to);
    switch(update.kind) {
      case INSERT: {
//        cout << "INSERT " << update.from << " -> " << update.to << endl;
        if(cfg->contains_edge(update.from, update.to))
          add_constraint(update.from, update.to, cfg->out_edges(update.from)->at(update.to));
        local_deps[dep.source].insert(dep.sink);
        _dependants[dep.source].insert(dep.sink);
        break;
      }
      case ERASE: {
//        cout << "ERASE " << update.from << " -> " << update.to << endl;
        remove_constraint(update.from, update.to);
        local_deps[dep.source].erase(dep.sink);
        _dependants[dep.source].erase(dep.sink);
        break;
      }
    }
  }

//  print_set(fp_pend_rm);
//
//  set_difference(fp_pend_new.begin(), fp_pend_new.end(), fp_pend_rm.begin(), fp_pend_rm.end(),
//      inserter(fixpoint_pending, fixpoint_pending.begin()));

  for(auto &deps : local_deps)
    for(auto dep : deps.second) {
      fixpoint_pending.erase(dep);
    }

//  print_set(fixpoint_pending);

  init_state();
}

std::ostream &::analysis::operator <<(std::ostream &out, analysis &_this) {
  _this.put(out);
  return out;
}
