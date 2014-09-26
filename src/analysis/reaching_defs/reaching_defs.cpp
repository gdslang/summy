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

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;

analysis::reaching_defs::reaching_defs::reaching_defs(class cfg *cfg) : analysis::analysis(cfg) {
  vector<set<tuple<size_t, function<lattice_elem()>>>> incoming;

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
      auto node_f = [&]() {
        lattice_elem elem = state[edge_it->first];
        nach unten, typ in incoming ändern
      };
//      incoming[edge_it->first].insert(make_tuple(node_id, transfer_f));
    }
  }
}

::analysis::lattice_elem *analysis::reaching_defs::reaching_defs::eval(size_t node) {
}
