/*
 * ip_propagator.cpp
 *
 *  Created on: Sep 21, 2014
 *      Author: jucs
 */

#include <summy/transformers/ip_propagator.h>
#include <summy/cfg/edge.h>
#include <summy/cfg/bfs_iterator.h>
#include <cppgdsl/rreil/copy_visitor.h>
#include <cppgdsl/rreil/rreil.h>

using namespace cfg;
using namespace gdsl::rreil;

void ip_propagator::transform() {
  for(auto node : *cfg) {
    auto &edges = *cfg->out_edges(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end();) {
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        copy_visitor cv;
        cv._([&](lin_var lv) {

        });
      });
      ev._([&](cond_edge *edge) {

      });
      edge_it->second->accept(ev);

    }
  }
}
