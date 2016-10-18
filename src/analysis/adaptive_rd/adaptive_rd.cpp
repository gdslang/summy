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
 * adaptive_rd.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/adaptive_rd/adaptive_rd.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/tools/rreil_util.h>
#include <summy/rreil/copy_visitor.h>
#include <cppgdsl/rreil/rreil.h>
#include <vector>
#include <set>
#include <functional>
#include <memory>
#include <assert.h>
#include <summy/cfg/edge/phi_edge.h>
#include <tuple>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;
using namespace analysis::adaptive_rd;
using analysis::liveness::liveness_result;
namespace sr = summy::rreil;

void adaptive_rd::add_constraint(size_t from, size_t to, const edge *e) {
  auto cleanup_live = [=](shared_ptr<adaptive_rd_state> acc) {
    return shared_ptr<adaptive_rd_state>(acc->remove([&](shared_ptr<id> id, singleton_value_t v) {
      return !lv_result.contains(to, id, 0, 64);
    }));
  };
  function<shared_ptr<adaptive_rd_state>()> transfer_f = [=]() {
//    cout << "default handler for edge " << node_id << "->" << dest_node << ", input state: " << *state[node_id] << endl;
      return cleanup_live(state[from]);
    };
  auto id_assigned = [&](int_t size, variable const *v) {
    sr::copy_visitor cv;
    v->get_id().accept(cv);
    shared_ptr<id> id_ptr(cv.retrieve_id());
    transfer_f = [=]() {
//    cout << "assignment handler for edge " << node_id << "->" << dest_node << ", input state: " << *state[node_id] << endl;
      auto acc = state[from];
      if(lv_result.contains(to, id_ptr, v->get_offset(), size)) {
        acc = shared_ptr<adaptive_rd_state>(acc->remove(id_set_t {id_ptr}));
        acc = shared_ptr<adaptive_rd_state>(acc->add( {singleton_t(id_ptr, to)}));
      }
      return cleanup_live(acc);
    };
  };
  
  edge_visitor ev;
  ev._([&](const stmt_edge *edge) {
    statement *stmt = edge->get_stmt();
    statement_visitor v;
    v._([&](assign const *a) {
      id_assigned(a->get_size(), &a->get_lhs());
    });
    v._([&](load const *l) {
      id_assigned(l->get_size(), &l->get_lhs());
    });
    v._([&](store const *s) {
      transfer_f = [=]() {
        return shared_ptr<adaptive_rd_state>(state[from]->set_memory_rev(to));
      };
    });
    stmt->accept(v);
  });
  ev._([&](phi_edge const *edge) {
    for(auto const &ass : edge->get_assignments()) {
      id_assigned(ass.get_size(), &ass.get_lhs());
    }
  });
  e->accept(ev);

  auto transfer_f_with_se = [=]() {
    auto in_state = transfer_f();
    this->in_states[to][from] = in_state;
    return in_state;
  };
  (constraints[to])[from] = transfer_f_with_se;
}

void analysis::adaptive_rd::adaptive_rd::remove_constraint(size_t from, size_t to) {
  (constraints[to]).erase(from);
  (in_states[to]).erase(from);
}

analysis::dependency analysis::adaptive_rd::adaptive_rd::gen_dependency(size_t from, size_t to) {
  return dependency { from, to };
}

void analysis::adaptive_rd::adaptive_rd::init_state() {
  size_t old_size = state.size();
  state.resize(cfg->node_count());
  for(size_t i = old_size; i < cfg->node_count(); i++) {
    if(fixpoint_pending.find(i) != fixpoint_pending.end()) state[i] = dynamic_pointer_cast<adaptive_rd_state>(
        start_value());
    else state[i] = dynamic_pointer_cast<adaptive_rd_state>(bottom());
  }
  in_states.resize(cfg->node_count());
}

adaptive_rd::adaptive_rd::adaptive_rd(class cfg *cfg, liveness_result lv_result) :
    fp_analysis(cfg), in_states(cfg->node_count()), lv_result(lv_result) {
  init();
}

analysis::adaptive_rd::adaptive_rd::~adaptive_rd() {
}

shared_ptr<analysis::domain_state> adaptive_rd::adaptive_rd::bottom() {
  return shared_ptr<adaptive_rd_state>(new adaptive_rd_state(false, elements_t(), 0));
}

shared_ptr<analysis::domain_state> adaptive_rd::adaptive_rd::start_value() {
  return shared_ptr<adaptive_rd_state>(new adaptive_rd_state(true, elements_t(), 0));
}

shared_ptr<::analysis::domain_state> adaptive_rd::adaptive_rd::get(size_t node) {
  return state[node];
}

void adaptive_rd::update(size_t node, shared_ptr<::analysis::domain_state> state) {
  this->state[node] = dynamic_pointer_cast<adaptive_rd_state>(state);
}

adaptive_rd_result analysis::adaptive_rd::adaptive_rd::result() {
  return adaptive_rd_result(state, in_states);
}

void analysis::adaptive_rd::adaptive_rd::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++) {
    out << i << ": " << *state[i];
    auto &node_in = in_states[i];
    cout << "; ";
    bool first = true;
    for(auto &state : node_in) {
      if(!first)
        cout << ", ";
      cout << state.first << " -> " << *state.second;
      first = false;
    }
    cout << endl;
  }
}
