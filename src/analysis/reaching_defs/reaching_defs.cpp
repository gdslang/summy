/*
 * reaching_defs.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/reaching_defs/reaching_defs.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge.h>
#include <cppgdsl/rreil/rreil.h>
#include <cppgdsl/rreil/copy_visitor.h>
#include <vector>
#include <set>
#include <functional>
#include <memory>
#include <assert.h>
#include <tuple>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;
using namespace analysis::reaching_defs;

#include <iostream>
analysis::reaching_defs::reaching_defs::reaching_defs(class cfg *cfg) : analysis::analysis(cfg) {
  state = state_t(cfg->node_count());
  for(size_t i = 0; i < state.size(); i++)
    state[i] = bottom();

  auto incoming = vector<vector<function<::analysis::reaching_defs::lattice_elem*()>>>(cfg->node_count());
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edges(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t dest_node = edge_it->first;
      function<::analysis::reaching_defs::lattice_elem*()> transfer_f = [=]() {
        return new lattice_elem(*state[node_id]);
      };
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        v._([&](assign *i) {
          cout << node_id << ": " << *i << endl;
          copy_visitor cv;
          i->get_lhs()->get_id()->accept(cv);
          shared_ptr<id> id_ptr(cv.get_id());
          transfer_f = [=]() {
            copy_visitor cv;
            id_ptr->accept(cv);
            return state[node_id]->add(definitions_t{make_tuple(dest_node, cv.get_id())});
          };
        });
        stmt->accept(v);
      });
      edge_it->second->accept(ev);
      incoming[dest_node].push_back(transfer_f);
    }
  }

  assert(incoming.size() == cfg->node_count());

  for(size_t i = 0; i < incoming.size(); i++) {
    vector<function<::analysis::reaching_defs::lattice_elem*()>> i_inc = incoming[i];
    auto node_f = [=]() {
      lattice_elem *elem = new lattice_elem(*this->state[i]);
      for(auto transfer_f : i_inc) {
        unique_ptr<lattice_elem> calc(transfer_f());
        elem = calc->lub(elem);
      }
      return elem;
    };
    constraints.push_back(node_f);
  }
}

lattice_elem *analysis::reaching_defs::reaching_defs::bottom() {
    return new ::analysis::reaching_defs::lattice_elem(definitions_t{});
}

lattice_elem *analysis::reaching_defs::reaching_defs::eval(size_t node) {
  return constraints[node]();
}

std::queue<size_t> analysis::reaching_defs::reaching_defs::initial() {
  /*
   * Todo fix
   */
  queue<size_t> foo;
  foo.push(0);
  return foo;
}

lattice_elem *analysis::reaching_defs::reaching_defs::get(size_t node) {
  return state[node];
}

void analysis::reaching_defs::reaching_defs::update(size_t node, ::analysis::lattice_elem *state) {
  delete this->state[node];
  this->state[node] = dynamic_cast<lattice_elem*>(state);
}

std::set<size_t> analysis::reaching_defs::reaching_defs::dependants(size_t node_id) {
  /*
   * Todo: fix
   */
//  if(node_id < cfg->node_count() - 1)
//    return set<size_t>{node_id + 1};
  return set<size_t>{(node_id + 1) % cfg->node_count()};
}

std::ostream &analysis::reaching_defs::operator <<(std::ostream &out, reaching_defs &_this) {
  for(size_t i = 0; i < _this.state.size(); i++) {
    out << i << ": " << *_this.state[i] << endl;
  }
  return out;
}
