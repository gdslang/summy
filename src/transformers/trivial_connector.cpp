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
 * trivial_connector.cpp
 *
 *  Created on: Sep 22, 2014
 *      Author: Julian Kranz
 */

#include <summy/transformers/trivial_connector.h>
#include <cppgdsl/rreil/copy_visitor.h>
#include <summy/cfg/node/node_visitor.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/cfg/node/node.h>
#include <cppgdsl/rreil/rreil.h>
#include <vector>
#include <queue>
#include <tuple>
#include <assert.h>
#include <summy/cfg/node/address_node.h>
#include <summy/tools/rreil_util.h>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;

void trivial_connector::update_address_node_map() {
  for(auto node : cfg_view) {
    node_visitor nv;
    nv._([&](address_node *sn) {
      address_node_map[sn->get_address()] = sn->get_id();
    });
    node->accept(nv);
  }
}

std::set<size_t> trivial_connector::transform_ur() {
  std::set<size_t> unresolved;

  update_address_node_map();
  queue<tuple<size_t, int_t>> branches;
  queue<tuple<size_t, sexpr*, bool, address*>> cond_branches;

  /*
   * Collect branch sites to be replaced
   */
  for(auto node : cfg_view) {
    auto &edges = *cfg->out_edge_payloads(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      bool replace = false;
      edge_visitor ev;
      ev._([&](const stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        v._([&](branch const *i) {
          switch(i->get_hint()) {
            case gdsl::rreil::BRANCH_HINT_CALL: {
              return;
            }
            case gdsl::rreil::BRANCH_HINT_RET: {
              return;
            }
            case gdsl::rreil::BRANCH_HINT_JUMP: {
              break;
            }
          }
          rreil_evaluator rev;
          bool evalable;
          int_t value;
          tie(evalable, value) = rev.evaluate(&i->get_target().get_lin());
          if(evalable) {
            replace = true;
            branches.push(make_tuple(edge_it->first, value));
          } else
            unresolved.insert(edge_it->first);
        });
        v._([&](cbranch const *i) {
          replace = true;
          auto branch = [&](bool then, address const *branch) {
            copy_visitor cv;
            i->get_cond().accept(cv);
            sexpr *cond = cv.retrieve_sexpr().release();
            branch->accept(cv);
            cond_branches.push(make_tuple(edge_it->first, cond, then, cv.retrieve_address().release()));
          };
          branch(true, &i->get_target_true());
          branch(false, &i->get_target_false());
        });
        stmt->accept(v);
      });
      edge_it->second->accept(ev);
      if(replace)
        cfg->update_destroy_edge(node->get_id(), edge_it->first, new edge());
    }
  }

  auto ip_assign = [&](int_t value) {
    return make_assign(64, make_variable(make_id("IP"), 0),
        make_expr(make_sexpr(make_linear(value)))).release();
  };

  auto dst_node = [&](int_t addr) {
    auto address_node_it = address_node_map.find(addr);
    if(address_node_it == address_node_map.end()) {
      size_t new_addr_node = cfg->create_node([&](size_t id) {
        return new address_node(id, addr, DECODABLE);
      });
      address_node_map[addr] = new_addr_node;
      return new_addr_node;
    } else
      return address_node_it->second;
  };

  /*
   * Replace single-destination branches
   */
  while(!branches.empty()) {
    size_t node_id;
    int_t addr;
    tie(node_id, addr) = branches.front();
    branches.pop();

    size_t dest_node_id = dst_node(addr);

    auto _ip_assign = ip_assign(addr);
    cfg->update_edge(node_id, dest_node_id, new stmt_edge(_ip_assign));
    delete _ip_assign;
  }

  /*
   * Replace conditional branches
   */
  while(!cond_branches.empty()) {
    size_t node_id;
    bool positive;
    sexpr *cond;
    address *addr;
    tie(node_id, cond, positive, addr) = cond_branches.front();
    cond_branches.pop();

    rreil_evaluator rev;
    bool evalable;
    int_t addr_value;
    tie(evalable, addr_value) = rev.evaluate(&addr->get_lin());

    auto preserve_branch = [&]() {
      size_t cond_end_node_id = cfg->create_node([&](size_t id) {
        return new (class node)(id);
      });

      cfg->update_edge(node_id, cond_end_node_id, new cond_edge(cond, positive));

      copy_visitor cv;
      addr->accept(cv);
      statement *branch = new class branch(cv.retrieve_address(), gdsl::rreil::BRANCH_HINT_JUMP);

      size_t dest_node_id = cfg->create_node([&](size_t id) {
        return new (class node)(id);
      });
      unresolved.insert(dest_node_id);

      cfg->update_edge(cond_end_node_id, dest_node_id, new stmt_edge(branch));

      delete branch;
    };

    if(evalable) {
      size_t dest_node_id = dst_node(addr_value);

      size_t cond_end_node_id = cfg->create_node([&](size_t id) {
        return new (class node)(id);
      });

      cfg->update_edge(node_id, cond_end_node_id, new cond_edge(cond, positive));

      auto _ip_assign = ip_assign(addr_value);
      cfg->update_edge(cond_end_node_id, dest_node_id, new stmt_edge(_ip_assign));
      delete _ip_assign;
    } else
      preserve_branch();

    delete cond;
    delete addr;
  }

  return unresolved;
}

void trivial_connector::transform() {
  transform_ur();
}
