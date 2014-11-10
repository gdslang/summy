/*
 * ismt.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/ismt/ismt.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <cppgdsl/rreil/rreil.h>
#include <iostream>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;
namespace sr = summy::rreil;

void analysis::ismt::analyse(size_t from) {
  cfg_view cv(cfg, from, true);
  for(auto node : cv) {
    size_t node_id = node->get_id();
    auto &edges = cfg->in_edges(node->get_id());
    for(auto from = edges.begin(); from != edges.end(); from++) {
      auto _edge = cfg->out_edges(*from)->at(node_id);
      edge_visitor ev;
      ev._([&](const stmt_edge *sedge) {
        statement *stmt = sedge->get_stmt();
        statement_visitor sv;
        sv._([&](assign *ass) {
          bool live = false;
          sr::visitor srv;
          srv._default([&](id *_id) {
            shared_ptr<id> lhs_id_wrapped(ass->get_lhs()->get_id(), [&](void *x) {});
            if(lv_result.contains(node_id, lhs_id_wrapped, 0, 64))
              live = true;
          });
          ass->accept(srv);
          shared_ptr<id> lhs_id_wrapped(ass->get_lhs()->get_id(), [&](void *x) {});
          if(live)
            try {
              cout << *stmt << ": " << smtb->build(stmt) << endl;
            } catch(string &s) {
              cout << s << endl;
            }
        });
        stmt->accept(sv);
      });
      _edge->accept(ev);
    }
  }
}
