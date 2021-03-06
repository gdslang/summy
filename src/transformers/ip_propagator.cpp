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
 * ip_propagator.cpp
 *
 *  Created on: Sep 21, 2014
 *      Author: jucs
 */

#include <summy/transformers/ip_propagator.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node.h>
#include <summy/cfg/node/node_visitor.h>
#include <summy/tools/rreil_util.h>

#include <cppgdsl/rreil/copy_visitor.h>
#include <cppgdsl/rreil/rreil.h>
#include <vector>
#include <functional>
#include <tuple>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;

std::tuple<bool, int_t> ip_propagator::evaluate(int_t ip_value, gdsl::rreil::expr const *e) {
  rreil_evaluator re([&](variable const *v) -> tuple<bool, int_t> {
    return make_tuple(rreil_prop::is_ip(v), ip_value);
  });
  return re.evaluate(e);
}

std::vector<int_t> *ip_propagator::analyze_ip() {
  vector<int_t> *result = new vector<int_t>(cfg->node_count());
  vector<bool> *calculated = new vector<bool>(cfg->node_count(), false);
  for(auto node : cfg_view) {
    size_t id = node->get_id();
    node_visitor nv;
    nv._([&](address_node *sn) {
      (*calculated)[id] = true;
      (*result)[id] = sn->get_address();
    });
    node->accept(nv);
    if(!(*calculated)[id])
      throw string("Unknown IP value");
    int_t ip_current = (*result)[id];
    auto &edges = *cfg->out_edge_payloads(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      edge_visitor ev;
      ev._([&](const stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        v._([&](assign const *i) {
          if(rreil_prop::is_ip(&i->get_lhs())) {
            bool evalable;
            tie(evalable, ip_current) = evaluate(ip_current, &i->get_rhs());
            if(!evalable)
              throw string("Can't evaluate IP value :-(");
          }
        });
        stmt->accept(v);
      });
      edge_it->second->accept(ev);

      //class node *child = cfg->get_node(edge_it->first);
      size_t child_id = edge_it->first;
      if((*calculated)[child_id]) {
        if((*result)[child_id] != ip_current)
          throw string("There should be no different IP values for one node :-(...");
      }
      else {
        (*result)[child_id] = ip_current;
        (*calculated)[child_id] = true;
      }
    }
  }
  delete calculated;
  return result;
}

void ip_propagator::transform() {
  auto ips = analyze_ip();

//  vector<node*> call_dest_replacement;

  for(auto node : cfg_view) {
    auto &edges = *cfg->out_edge_payloads(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      edge_visitor ev;
      ev._([&](const stmt_edge *edge) {
        statement_visitor sv;
        sv._([&](branch const *b) {
          if(b->get_hint() == gdsl::rreil::BRANCH_HINT_CALL) {
            size_t an_id = cfg->create_node([&](size_t id) {
              return new address_node(id, (*ips)[edge_it->first], speculative_decoding ? DECODABLE : UNDEFINED);
            });
            cfg->update_edge(edge_it->first, an_id, new class call_edge(false));
          }
        });
        edge->get_stmt()->accept(sv);

        copy_visitor cv;
        cv._([&](std::unique_ptr<variable> v) {
          if(rreil_prop::is_ip(v.get())) {
            return make_linear((*ips)[node->get_id()]);
          } else
            return make_linear(std::move(v));
        });
        edge->get_stmt()->accept(cv);
        auto stmt_mod = cv.retrieve_statement();
        cfg->update_destroy_edge(node->get_id(), edge_it->first, new stmt_edge(stmt_mod.get()));
      });
      ev._([&](const cond_edge *edge) {
        /*
         * Todo
         */
      });
      edge_it->second->accept(ev);
    }
  }

//  for(node *n : call_dest_replacement) {
//    cfg->replace_node_payload(n);
//  }

  delete ips;
}
