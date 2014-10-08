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
#include <cppgdsl/rreil/copy_visitor.h>
#include <map>

#include <stdio.h>

using namespace std;
using namespace gdsl::rreil;
using namespace cfg;

void decomposer::transform() {
  for(auto node : *cfg) {
//      printf("Next node...\n");
    auto &edges = *cfg->out_edges(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end();) {
//      printf("Next edge...\n");
      size_t edge_dst_node = edge_it->first;
      bool replace = false;
      edge *replacement = NULL;
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        v._([&](ite *i) {
          replace = true;
          auto branch = [&](vector<statement*> *branch, bool positive) {
            size_t branch_node_id = cfg->create_node([&](size_t id) {
              return new (class node)(id);
            });
            size_t last_branch = cfg->add_nodes(branch, branch_node_id);

            edges[branch_node_id] = new cond_edge(i->get_cond(), positive);
            auto &branch_last_out_edges = *cfg->out_edges(last_branch);
            branch_last_out_edges[edge_dst_node] = new (class edge)();
          };
          branch(i->get_then_branch(), true);
          branch(i->get_else_branch(), false);
        });
        v._([&](_while *w) {
          size_t body_node_id = cfg->create_node([&](size_t id) {
            return new (class node)(id);
          });
          size_t last_body = cfg->add_nodes(w->get_body(), body_node_id);

          edges[body_node_id] = new cond_edge(w->get_cond(), true);

          replace = true;
          replacement = new cond_edge(w->get_cond(), false);

          auto &body_last_out_edges = *cfg->out_edges(last_body);
          body_last_out_edges[node->get_id()] = new (class edge)();
        });
        stmt->accept(v);
      });
      edge_it->second->accept(ev);

      auto edge_it_old = edge_it++;
      if(replace) {
        delete edge_it_old->second;
        if(replacement == NULL)
          edges.erase(edge_it_old);
        else
          edge_it_old->second = replacement;
      }
    }
//    printf("id: %zu\n", a->get_id());
  }
}
