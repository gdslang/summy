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

using namespace analysis;
using namespace cfg;
using namespace std;

void ::analysis::analysis::init_fixpoint_initial() {
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

  init_fixpoint_initial();
}

void ::analysis::analysis::update(vector<struct update> &updates) {
  for(auto &update : updates) {
    switch(update.kind) {
      case INSERT: {
        add_constraint(update.from, update.to, cfg->out_edges(update.from)->at(update.to));
        add_dependency(update.from, update.to);
        break;
      }
      case ERASE: {
        remove_constraint(update.from, update.to);
        remove_dependency(update.from, update.to);
        break;
      }
    }
  }
}

std::ostream &::analysis::operator <<(std::ostream &out, analysis &_this) {
  _this.put(out);
  return out;
}
