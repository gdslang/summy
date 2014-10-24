/*
 * phi_inserter.cpp
 *
 *  Created on: Oct 20, 2014
 *      Author: Julian Kranz
 */

#include <summy/transformers/ssa/phi_inserter.h>
#include <summy/cfg/phi_edge.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/analysis/adaptive_rd/adaptive_rd.h>
#include <summy/analysis/adaptive_rd/adaptive_rd_elem.h>
#include <cppgdsl/rreil/rreil.h>
#include <cppgdsl/rreil/copy_visitor.h>
#include <vector>
#include <iostream>
#include <assert.h>

using namespace std;
using namespace cfg;
using namespace analysis::adaptive_rd;
using namespace gdsl::rreil;

void phi_inserter::transform() {
  struct phi_task {
    phi_edge *pe;
    size_t from;
    size_t to;
  };

  vector<phi_task> tasks;

  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edges(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t edge_dst_node = edge_it->first;

      shared_ptr<adaptive_rd_elem> dst_incoming = (rd_result.in_states[edge_dst_node])[node_id];
      elements_t const &dst_incoming_elements = dst_incoming->get_elements();
      shared_ptr<adaptive_rd_elem> dst_state = rd_result.result[edge_dst_node];

      assignments_t phi_assignments;
      for(auto &mapping : dst_state->get_elements()) {
        auto mapping_inc = dst_incoming_elements.find(mapping.first);
        auto add_phi = [&]() {
          copy_visitor cv;
          mapping.first->accept(cv);
          variable v(cv.get_id(), 0);
          phi_assignments.push_back(phi_assign(&v, &v, 64));
        };
        if(mapping_inc == dst_incoming_elements.end()) {
          /*
           * Incoming undefined value
           */
          add_phi();
        } else {
          if(mapping.second != mapping_inc->second) {
            /*
             * Different defs incoming => phi edge required
             */
            add_phi();
          }
        }
      }

      if(phi_assignments.size() > 0)
        tasks.push_back({ new phi_edge(phi_assignments), node_id, edge_dst_node });
    }
  }

  for(auto &task : tasks) {
    size_t interm_node_id = cfg->create_node([&](size_t id) {
      return new (class node)(id);
    });

    auto const &from_out_edges = *cfg->out_edges(task.from);
    cfg->update_edge(task.from, interm_node_id, from_out_edges.at(task.to));
    cfg->erase_edge(task.from, task.to);

    cfg->update_edge(interm_node_id, task.to, task.pe);

    /*
     * Todo: the following is awkwardly hacky and totally wrong
     */
//    assert(interm_node_id == rd_result.result.size());
//    rd_result.result.push_back(rd_result.in_states[task.to][task.from]);
  }
}
