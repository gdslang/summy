/*
 * analysis.cpp
 *
 *  Created on: Oct 15, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/analysis.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/bfs_iterator.h>
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
}

void ::analysis::analysis::update(vector<struct update> &updates) {
  fixpoint_pending.clear();

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

  set<size_t> fp_pend_rm;
  for(auto &update : updates) {
    switch(update.kind) {
      case INSERT: {
        add_constraint(update.from, update.to, cfg->out_edges(update.from)->at(update.to));
        fp_pend_rm.insert(add_dependency(update.from, update.to));
        break;
      }
      case ERASE: {
        remove_constraint(update.from, update.to);
        fp_pend_rm.erase(remove_dependency(update.from, update.to));
        break;
      }
    }
  }

  fixpoint_pending.erase(fp_pend_rm.begin(), fp_pend_rm.end());
}

std::ostream &::analysis::operator <<(std::ostream &out, analysis &_this) {
  _this.put(out);
  return out;
}
