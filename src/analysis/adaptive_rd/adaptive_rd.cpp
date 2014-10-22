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
#include <summy/cfg/edge.h>
#include <summy/cfg/phi_edge.h>
#include <summy/cfg/edge_visitor.h>
#include <summy/tools/rreil_util.h>
#include <summy/rreil/copy_visitor.h>
#include <cppgdsl/rreil/rreil.h>
#include <vector>
#include <set>
#include <functional>
#include <memory>
#include <assert.h>
#include <tuple>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;
using namespace analysis::adaptive_rd;
using analysis::liveness::liveness_result;
namespace sr = summy::rreil;

void adaptive_rd::init_constraints() {
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edges(node_id);
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t dest_node = edge_it->first;
      auto cleanup_live = [=](shared_ptr<adaptive_rd_elem> acc) {
        return shared_ptr<adaptive_rd_elem>(acc->remove([&](shared_ptr<id> id, singleton_value_t v) {
          return !lv_result->contains(dest_node, id, 0, 64);
        }));
      };
      function<shared_ptr<adaptive_rd_elem>()> transfer_f = [=]() {
//        cout << "default handler for edge " << node_id << "->" << dest_node << ", input state: " << *state[node_id] << endl;
        return cleanup_live(state[node_id]);
      };
      auto id_assigned = [&](int_t size, variable *v) {
        sr::copy_visitor cv;
        v->get_id()->accept(cv);
        shared_ptr<id> id_ptr(cv.get_id());
       transfer_f = [=]() {
//           cout << "assignment handler for edge " << node_id << "->" << dest_node << ", input state: " << *state[node_id] << endl;
          auto acc = shared_ptr<adaptive_rd_elem>(state[node_id]->remove(id_set_t { id_ptr }));
          if(lv_result->contains(dest_node, id_ptr, v->get_offset(), size))
            acc = shared_ptr<adaptive_rd_elem>(acc->add({singleton_t(id_ptr, dest_node)}));
          return cleanup_live(acc);
        };
      };
      edge_visitor ev;
      ev._([&](const stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        v._([&](assign *a) {
          id_assigned(rreil_prop::size_of_assign(a), a->get_lhs());
        });
        v._([&](load *l) {
          id_assigned(l->get_size(), l->get_lhs());
        });
        stmt->accept(v);
      });
      ev._([&](const phi_edge *edge) {
        for(auto const &ass : edge->get_assignments()) {
          id_assigned(ass.get_size(), ass.get_lhs());
        }
      });
      edge_it->second->accept(ev);
      (constraints[dest_node])[node_id] = transfer_f;
    }
  }

  assert(constraints.size() == cfg->node_count());
}

void adaptive_rd::init_dependants() {
  for(auto node : *cfg) {
    auto &edges = *cfg->out_edges(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      _dependants[node->get_id()].insert(edge_it->first);
    }
  }
}

adaptive_rd::adaptive_rd::adaptive_rd(class cfg *cfg, liveness_result *lv_result) :
    analysis::analysis(cfg), lv_result(lv_result) {
  init();

  state = state_t(cfg->node_count());
  for(size_t i = 0; i < state.size(); i++)
    if(fixpoint_initial.find(i) != fixpoint_initial.end()) state[i] = dynamic_pointer_cast<adaptive_rd_elem>(start_value());
    else state[i] = dynamic_pointer_cast<adaptive_rd_elem>(bottom());
}

analysis::adaptive_rd::adaptive_rd::~adaptive_rd() {
  delete lv_result;
}

shared_ptr<analysis::lattice_elem> adaptive_rd::adaptive_rd::bottom() {
    return shared_ptr<adaptive_rd_elem>(new adaptive_rd_elem(false, elements_t()));
}

shared_ptr<analysis::lattice_elem> adaptive_rd::adaptive_rd::start_value() {
    return shared_ptr<adaptive_rd_elem>(new adaptive_rd_elem(true, elements_t()));
}

shared_ptr<::analysis::lattice_elem> adaptive_rd::adaptive_rd::get(size_t node) {
  return state[node];
}

void adaptive_rd::update(size_t node, shared_ptr<::analysis::lattice_elem> state) {
  this->state[node] = dynamic_pointer_cast<adaptive_rd_elem>(state);
}

adaptive_rd_result *analysis::adaptive_rd::adaptive_rd::result() {
  auto in_states = adaptive_rd_result::in_states_t(cfg->node_count());
  for(size_t i = 0; i < cfg->node_count(); i++)
    for(auto &edge_c : constraints[i])
      (in_states[i])[edge_c.first] = dynamic_pointer_cast<adaptive_rd_elem>(edge_c.second());
  return new adaptive_rd_result(state, in_states);
}

void analysis::adaptive_rd::adaptive_rd::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++)
    out << i << ": " << *state[i] << endl;
}
