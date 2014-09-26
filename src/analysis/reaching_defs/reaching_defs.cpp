/*
 * reaching_defs.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/reaching_defs/reaching_defs.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge.h>
#include <cppgdsl/rreil/rreil.h>
#include <cppgdsl/rreil/copy_visitor.h>
#include <vector>
#include <set>
#include <functional>
#include <memory>
#include <assert.h>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;
using namespace analysis::reaching_defs;


lattice_elem *analysis::reaching_defs::reaching_defs::bottom() {
    return new ::analysis::reaching_defs::lattice_elem(set<id*>{});
}

analysis::reaching_defs::reaching_defs::reaching_defs(class cfg *cfg) : analysis::analysis(cfg) {
  state = state_t(cfg->node_count(), bottom());

  auto incoming = vector<vector<function<::analysis::reaching_defs::lattice_elem*()>>>(cfg->node_count());
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edges(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      function<::analysis::reaching_defs::lattice_elem*()> transfer_f = [=]() {
        return state[node_id];
      };
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        v._([&](assign *i) {
          copy_visitor cv;
          i->get_lhs()->get_id()->accept(cv);
          shared_ptr<id> id_ptr(cv.get_id());
          transfer_f = [=]() {
            copy_visitor cv;
            id_ptr->accept(cv);
            return state[node_id]->add(set<id*>{cv.get_id()});
          };
        });
        stmt->accept(v);
      });
      edge_it->second->accept(ev);
      incoming[edge_it->first].push_back(transfer_f);
    }
  }

  assert(incoming.size() == cfg->node_count());

  for(size_t i = 0; i < incoming.size(); i++) {
    vector<function<::analysis::reaching_defs::lattice_elem*()>> i_inc = incoming[i];
    auto node_f = [=]() {
      lattice_elem *elem = new lattice_elem(*this->state[i]);
      for(auto transfer_f : i_inc) {
        unique_ptr<lattice_elem> calc(transfer_f());
        elem = calc->lub(elem);
      }
      return elem;
    };
    constraints.push_back(node_f);
  }
}



::analysis::lattice_elem *analysis::reaching_defs::reaching_defs::eval(size_t node) {
}
