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
#include <map>

namespace a = analysis;
using namespace cfg;
using namespace std;

set<size_t> a::analysis::roots(set<size_t> const &all, const dependants_t &dep_dants) {
  set<size_t >result;
  set<size_t> left = all;

  while(!left.empty()) {
    size_t next;
    auto it = left.begin();
    next = *it;
    left.erase(it);

    result.insert(next);
    queue<size_t> bfs_q;
    bfs_q.push(next);
    while(!bfs_q.empty()) {
      size_t node = bfs_q.front();
      bfs_q.pop();
      auto dd_it = dep_dants.find(node);
      if(dd_it != dep_dants.end())
        for(auto dep_dant : dd_it->second) {
          bfs_q.push(dep_dant);
          left.erase(dep_dant);
        }
    }
  }
  return result;
}

void ::analysis::analysis::init_fixpoint_pending() {
  for(size_t i = 0; i < cfg->node_count(); i++)
    fixpoint_pending.insert(i);

  fixpoint_pending = roots(fixpoint_pending, _dependants);
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

  fixpoint_pending.clear();

  for(auto &update : updates) {
    switch(update.kind) {
      case UPDATE:
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
    auto insert = [&]() {
      local_deps[dep.source].insert(dep.sink);
      _dependants[dep.source].insert(dep.sink);
    };
    auto erase = [&]() {
      local_deps[dep.source].erase(dep.sink);
      _dependants[dep.source].erase(dep.sink);
    };
    switch(update.kind) {
      case UPDATE: {
        erase();
        insert();
        break;
      }
      case INSERT: {
        insert();
        break;
      }
      case ERASE: {
        erase();
        break;
      }
    }
  }
  fixpoint_pending = roots(fixpoint_pending, local_deps);
  init_state();

  for(auto &update : updates) {
    auto insert = [&]() {
//      cout << "INSERT " << update.from << " -> " << update.to << endl;
      if(cfg->contains_edge(update.from, update.to))
        add_constraint(update.from, update.to, cfg->out_edges(update.from)->at(update.to));
    };
    auto erase = [&]() {
//      cout << "ERASE " << update.from << " -> " << update.to << endl;
      remove_constraint(update.from, update.to);
    };
    switch(update.kind) {
      case UPDATE: {
        erase();
        insert();
        break;
      }
      case INSERT: {
        insert();
        break;
      }
      case ERASE: {
        erase();
        break;
      }
    }
  }
}

std::ostream &::analysis::operator <<(std::ostream &out, analysis &_this) {
  _this.put(out);
  return out;
}
