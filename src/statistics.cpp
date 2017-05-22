/*
 * Copyright 2016 Julian Kranz, Technical University of Munich
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
 * statistics.cpp
 *
 *  Created on: May 20, 2016
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <cppgdsl/rreil/sexpr/sexpr.h>
#include <cppgdsl/rreil/sexpr/sexpr_cmp.h>

#include <cppgdsl/rreil/statement/branch.h>
#include <cppgdsl/rreil/statement/branch.h>
#include <cppgdsl/rreil/statement/cbranch.h>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/rreil/sexpr/sexpr_visitor.h>
#include <summy/statistics.h>
#include <summy/tools/rreil_util.h>

#include "cppgdsl/rreil/statement/statement_visitor.h"
#include <algorithm>
#include <istream>
#include <iterator>
#include <tuple>

using namespace std;
using namespace gdsl::rreil;

using analysis::node_targets_t;
using cfg::edge_visitor;
using cfg::stmt_edge;
using summy::rreil::sexpr_visitor;

/*
 * branch_statistics
 */

branch_statistics::branch_statistics(
  gdsl::gdsl &gdsl, analysis::summary_dstack &sd, cfg::jd_manager &jd_manager) {
  cfg::cfg *cfg = sd.get_cfg();

  node_targets_t const &node_targets = sd.get_targets();

  for(auto nt_it : node_targets) {
    size_t node_from = nt_it.first;
    cfg::edge_payloads_t const *edges = cfg->out_edge_payloads(node_from);
    assert(edges->size() == 1);
    cfg::edge const *e = edges->begin()->second;
    bool is_branch = false;
    bool is_direct;
    gdsl::rreil::branch_hint hint;
    edge_visitor ev;
    ev._([&](stmt_edge const *se) {
      statement_visitor v;
      v._([&](branch const *i) {
        is_branch = true;
        hint = i->get_hint();
        rreil_evaluator rev;
        tie(is_direct, ignore) = rev.evaluate(&i->get_target().get_lin());
      });
      se->get_stmt()->accept(v);
    });
    e->accept(ev);
    assert(is_branch);
    if(hint == gdsl::rreil::branch_hint::BRANCH_HINT_RET) continue;

    if(!is_direct) {
      size_t machine_address = jd_manager.machine_address_of(node_from);
      set<size_t> _new;
      for(auto address : nt_it.second) {
        int_t ip = gdsl.get_ip();
        bool seekable = !gdsl.seek(address);
        cout << hex << address << " " << dec << seekable << endl;
        assert(!gdsl.seek(ip));
        if(seekable) _new.insert(address);
      }

      auto current_it = address_targets.find(machine_address);
      if(current_it != address_targets.end()) {
        set<size_t> intersection;
        set_intersection(current_it->second.targets.begin(), current_it->second.targets.end(),
          _new.begin(), _new.end(), inserter(intersection, intersection.begin()));
        address_targets[machine_address].targets = intersection;

      } else
        address_targets[machine_address].targets = _new;

      address_targets[machine_address].is_call = hint == gdsl::rreil::branch_hint::BRANCH_HINT_CALL;

      if(address_targets[machine_address].targets.size() == 0)
        cout << "No targets for " << (address_targets[machine_address].is_call ? "call" : "jump")
             << " at 0x" << hex << machine_address << dec << endl;
    }
  }
}

branch_statistics_data_t branch_statistics::get_stats() {
  branch_statistics_data_t r;
  for(auto at_it : address_targets) {
    if(at_it.second.is_call)
      r.calls_total_indirect++;
    else
      r.jmps_total_indirect++;
    if(at_it.second.targets.size() > 0) {
      if(at_it.second.is_call)
        r.calls_with_targets++;
      else
        r.jmps_with_targets++;
    }
  }
  return r;
}

/*
 * condition_statistics
 */

condition_statistics_data_t condition_statistics::get_stats() {
  size_t total_conditions = 0;
  size_t cmp_conditions = 0;
  for(auto node : cfg) {
    size_t node_id = node->get_id();
    for(auto edge_it : *cfg.out_edge_payloads(node_id)) {
      bool is_cmp = false;
      cfg::edge const *e = edge_it.second;
      edge_visitor ev;
      ev._([&](stmt_edge const *se) {
        statement_visitor sv;
        sv._([&](cbranch const *cb) {
          sexpr const &sex = cb->get_cond();
          ::summy::rreil::sexpr_visitor sv;
          sv._([&](sexpr_cmp const *cmp) { is_cmp = true; });
          sex.accept(sv);
          total_conditions++;
        });
        se->get_stmt()->accept(sv);
      });
      e->accept(ev);
      if(is_cmp) cmp_conditions++;
    }
  }
  return condition_statistics_data_t{total_conditions, cmp_conditions};
}

/*
 * loc_statistics
 */

size_t loc_statistics::get_loc() {
  size_t loc = 0;
  for(auto node : cfg) {
    size_t node_id = node->get_id();
    for(auto edge_it : *cfg.out_edge_payloads(node_id)) {
      cfg::edge const *e = edge_it.second;
      edge_visitor ev;
      ev._([&](stmt_edge const *se) { loc++; });
      e->accept(ev);
    }
  }
  return loc;
}
