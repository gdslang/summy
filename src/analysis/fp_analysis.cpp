/*
 * fp_analysis.cpp
 *
 *  Created on: Oct 15, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/fp_analysis.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/bfs_iterator.h>
#include <algorithm>
#include <vector>
#include <set>
#include <map>

using namespace cfg;
using namespace std;
using namespace analysis;

set<size_t> fp_analysis::roots(set<size_t> const &all, const dependants_t &dep_dants) {
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
          auto left_it = left.find(dep_dant);
          if(left_it != left.end()) {
            bfs_q.push(dep_dant);
            left.erase(left_it);
          }
          result.erase(dep_dant);
        }
    }
  }

  return result;
}

void fp_analysis::init_fixpoint_pending() {
  for(size_t i = 0; i < cfg->node_count(); i++)
    fixpoint_pending.insert(i);

  fixpoint_pending = roots(fixpoint_pending, _dependants);
}

fp_analysis::fp_analysis(class cfg *cfg) :
    rec(cfg, false), cfg(cfg) {
}

void ::fp_analysis::fp_analysis::init() {
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edge_payloads(node_id);
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      add_constraint(node_id, edge_it->first, edge_it->second);
      auto dep = gen_dependency(node_id, edge_it->first);
      assert_dependency(dep);
    }
  }

  init_fixpoint_pending();
  init_state();
}

void fp_analysis::update(vector<struct update> const &updates) {
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
        add_constraint(update.from, update.to, cfg->out_edge_payloads(update.from)->at(update.to));
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

void analysis::fp_analysis::record_updates() {
  rec.start();
}

void analysis::fp_analysis::record_stop_commit() {
  rec.stop();
  update(rec.checkout_updates());
}

void analysis::fp_analysis::assert_dependency(dependency dep) {
  _dependants[dep.source].insert(dep.sink);
}

node_compare_t analysis::fp_analysis::get_fixpoint_node_comparer() {
  return [](size_t a, size_t b) {
    return a < b;
  };
}

std::ostream &operator <<(std::ostream &out, fp_analysis &_this) {
  _this.put(out);
  return out;
}
