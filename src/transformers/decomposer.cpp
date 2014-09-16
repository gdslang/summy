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
      statement *stmt = edge_it->second->get_stmt();
      statement_visitor v;
      v._([&](ite *i) {
//        del = true;

        cfg->add_nodes(i->get_then_branch(), a->get_id());

//        node *fiep = new node(cfg->next_node_id());
//        cfg->add_node(fiep);
//
//        edges[fiep->get_id()] = new edge(i->get_then_branch()->operator[](0));

        printf(":-)\n");
      });
      stmt->accept(v);

      if(del)
        edges.erase(edge_it++);
      else
        edge_it++;
    }

    printf("id: %zu\n", a->get_id());
  }
}
