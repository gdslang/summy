/*
 * decomposer.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: Julian Kranz
 */

#include <summy/transformers/decomposer.h>
#include <summy/cfg/edge.h>
#include <summy/cfg/node.h>
#include <summy/cfg/bfs_iterator.h>
#include <cppgdsl/rreil/rreil.h>
#include <cppgdsl/rreil/statement/statement_visitor.h>
#include <map>

#include <stdio.h>

using namespace gdsl::rreil;
using namespace cfg;

void decomposer::transform() {
  for(auto a : *cfg) {
    auto &edges = cfg->out_edges(a->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end();) {
      bool del = false;
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        v._([&](ite *i) {
          del = true;

          /*
           * then branch
           */

          node *then_node = new node(cfg->next_node_id());
          cfg->add_node(then_node);
          size_t last_then = cfg->add_nodes(i->get_then_branch(), then_node->get_id());
          edges[then_node->get_id()] = new cond_edge(i->get_cond(), true);

          auto &then_last_out_edges = cfg->out_edges(last_then);
          then_last_out_edges[edge_it->first] = new (class edge)();

          /*
           * else branch
           */

          node *else_node = new node(cfg->next_node_id());
          cfg->add_node(else_node);
          size_t last_else = cfg->add_nodes(i->get_else_branch(), else_node->get_id());
          edges[else_node->get_id()] = new cond_edge(i->get_cond(), false);

          auto &else_last_out_edges = cfg->out_edges(last_else);
          else_last_out_edges[edge_it->first] = new (class edge)();

          printf(":-)\n");
        });
        stmt->accept(v);
      });
      edge_it->second->accept(ev);
      if(del)
        edges.erase(edge_it++);
      else
        edge_it++;

    }

    printf("id: %zu\n", a->get_id());
  }
}
