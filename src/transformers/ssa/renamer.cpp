/*
 * renamer.cpp
 *
 *  Created on: Oct 20, 2014
 *      Author: Julian Kranz
 */

#include <summy/transformers/ssa/renamer.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/edge.h>
#include <summy/cfg/edge_visitor.h>
#include <summy/rreil/id/ssa_id.h>
#include <cppgdsl/rreil/copy_visitor.h>
#include <cppgdsl/rreil/rreil.h>
#include <tuple>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;

void renamer::transform() {
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edges(node->get_id());
    vector<tuple<size_t, edge*>> updates;
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t edge_dst_node = edge_it->first;
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        copy_visitor cv;
        cv._([&](id *_id, int_t offset) -> variable* {
          return new variable(new summy::rreil::ssa_id(_id, 99), offset);
        });
        edge->get_stmt()->accept(cv);
        updates.push_back(make_tuple(edge_it->first, new stmt_edge(cv.get_statement())));
      });
      edge_it->second->accept(ev);
    }
    for(auto &update : updates) {
      size_t dst_node;
      edge *stmt_edge;
      tie(dst_node, stmt_edge) = update;
      edges[dst_node] = stmt_edge;
    }
  }
}
