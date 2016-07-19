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
 * phi_inserter.cpp
 *
 *  Created on: Oct 20, 2014
 *      Author: Julian Kranz
 */

#include <summy/transformers/ssa/phi_inserter.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/analysis/adaptive_rd/adaptive_rd.h>
#include <summy/rreil/copy_visitor.h>
#include <cppgdsl/rreil/rreil.h>
#include <vector>
#include <iostream>
#include <assert.h>
#include <summy/analysis/adaptive_rd/adaptive_rd_state.h>
#include <summy/cfg/edge/phi_edge.h>

using namespace std;
using namespace cfg;
using namespace analysis::adaptive_rd;
namespace sr = summy::rreil;
using namespace gdsl::rreil;

void phi_inserter::task_from_edge(vector<phi_task> &tasks, size_t from, size_t to) {
  shared_ptr<adaptive_rd_state> dst_incoming = (rd_result.in_states[to])[from];
  elements_t const &dst_incoming_elements = dst_incoming->get_elements();
  shared_ptr<adaptive_rd_state> dst_state = rd_result.result[to];

  assignments_t phi_assignments;
  for(auto &mapping : dst_state->get_elements()) {
    auto mapping_inc = dst_incoming_elements.find(mapping.first);
    auto add_phi = [&]() {
      sr::copy_visitor cv;
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

  bool mem_phi = false;
  if(dst_state->get_memory_rev() != dst_incoming->get_memory_rev())
    mem_phi = true;

  if(phi_assignments.size() > 0 || mem_phi)
    tasks.push_back({ new phi_edge(phi_assignments, phi_memory(0, 0)), from, to });
}

void phi_inserter::transform(std::vector<phi_task> &tasks) {
  for(auto &task : tasks) {
    size_t interm_node_id = cfg->create_node([&](size_t id) {
      return new (class node)(id);
    });

    auto const &from_out_edges = *cfg->out_edge_payloads(task.from);
    cfg->update_edge(task.from, interm_node_id, from_out_edges.at(task.to));
    cfg->erase_edge(task.from, task.to);

    cfg->update_edge(interm_node_id, task.to, task.pe);
  }
}

void phi_inserter::transform() {
  vector<phi_task> tasks;
  for(auto node : *cfg) {
    size_t from = node->get_id();
    auto &edges = *cfg->out_edge_payloads(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t to = edge_it->first;
      task_from_edge(tasks, from, to);
    }
  }
  transform(tasks);
}

void phi_inserter::update(std::set<std::tuple<size_t, size_t>> &updates) {
  vector<phi_task> tasks;
  for(auto &update : updates) {
    size_t from;
    size_t to;
    tie(from, to) = update;
    task_from_edge(tasks, from, to);
  }
  transform(tasks);
}
