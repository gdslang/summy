/*
 * fp_analysis.cpp
 *
 *  Created on: Oct 15, 2014
 *      Author: Julian Kranz
 */

#include <algorithm>
#include <assert.h>
#include <map>
#include <set>
#include <summy/analysis/fp_analysis.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>
#include <vector>

using namespace cfg;
using namespace std;
using namespace analysis;

static size_t first_if_pair(size_t x) {
  return x;
}

static size_t first_if_pair(std::pair<const unsigned long, const ::cfg::edge *> const &p) {
  return p.first;
}

std::map<size_t, constraint_t> fp_analysis::constraints_at(size_t node) {
  std::map<size_t, constraint_t> r;
  auto inner = [&](const auto &edges) {
    if(edges.size() == 0) {
      std::shared_ptr<analysis::domain_state> state = start_state(node);
      r[node] = [=](size_t) { return default_context(state); };
    } else
      for(auto other : edges) {
        auto ss_other = first_if_pair(other);
        auto payload = get_cfg()->out_edge_payloads(ss_other)->at(node);
        r[ss_other] = [=](size_t ctx) { return transform(ss_other, node, payload, ctx); };
      }
  };
  if(direction == analysis_direction::FORWARD)
    inner(cfg->in_edges(node));
  else
    inner(*cfg->out_edge_payloads(node));
  return r;
}

set<analysis_node> fp_analysis::roots(
  set<analysis_node> const &all, const dependants_t &dep_dants) {
  set<analysis_node> result;
  set<analysis_node> left = all;

  while(!left.empty()) {
    auto it = left.begin();
    analysis_node next = *it;
    left.erase(it);

    result.insert(next);
    queue<analysis_node> bfs_q;
    bfs_q.push(next);
    while(!bfs_q.empty()) {
      analysis_node node = bfs_q.front();
      bfs_q.pop();
      auto dd_it = dep_dants.find(node.id);
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

fp_analysis::fp_analysis(class cfg *cfg, analysis_direction direction)
    : direction(direction), rec(cfg, false), cfg(cfg) {}

void ::fp_analysis::fp_analysis::init() {
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edge_payloads(node_id);
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
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

  //  fixpoint_pending.clear();

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
}

void analysis::fp_analysis::record_updates() {
  rec.start();
}

bool analysis::fp_analysis::record_stop_commit() {
  rec.stop();
  auto const &updates = rec.checkout_updates();
  update(updates);
  return updates.size() > 0;
}

void analysis::fp_analysis::clear_pending() {
  fixpoint_pending.clear();
}

void analysis::fp_analysis::assert_dependency(dependency dep) {
  assert(!(dep.source == 166 && dep.sink == 264));
  _dependants[dep.source].insert(dep.sink);
}

node_compare_t analysis::fp_analysis::get_fixpoint_node_comparer() {
  return [](analysis_node const &a, analysis_node const &b) { return a < b; };
}

std::ostream &operator<<(std::ostream &out, fp_analysis &_this) {
  _this.put(out);
  return out;
}

