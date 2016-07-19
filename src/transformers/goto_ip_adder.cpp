/*
 * Copyright 2014-2016 Julian Kranz, Technical University of Munich
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * goto_ip_adder.cpp
 *
 *  Created on: Sep 23, 2014
 *      Author: Julian Kranz
 */

#include <cppgdsl/rreil/rreil.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/cfg/node/node.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/transformers/goto_ip_adder.h>
#include <vector>
#include <queue>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;

void goto_ip_adder::transform() {
  vector<bool> dead_node = vector<bool>(cfg->node_count(), false);
  queue<node*> transform_queue;

  for(auto node : cfg_view) {
//      printf("Next node...\n");
    auto &edges = *cfg->out_edge_payloads(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      edge_visitor ev;
      ev._([&](const stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        v._([&](branch *i) {
              dead_node[edge_it->first] = true;
            });
        v._([&](cbranch *i) {
              dead_node[edge_it->first] = true;
            });
        stmt->accept(v);
      });
      edge_it->second->accept(ev);
    }
    if(edges.empty() && !dead_node[node->get_id()])
      transform_queue.push(node);
  }
  while(!transform_queue.empty()) {
    class node *node = transform_queue.front();
    transform_queue.pop();

    size_t new_node_id = cfg->create_node([&](size_t id) {
      return new (class node)(id);
    });

    statement *goto_ip = new branch(new address(64, new lin_var(new variable(new arch_id("IP"), 0))),
        gdsl::rreil::BRANCH_HINT_JUMP);

    cfg->update_edge(node->get_id(), new_node_id, new stmt_edge(goto_ip));
    delete goto_ip;
  }
}
