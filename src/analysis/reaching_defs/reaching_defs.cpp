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
    state[i] = dynamic_pointer_cast<lattice_elem>(bottom());

  auto incoming = vector<vector<function<shared_ptr<::analysis::reaching_defs::lattice_elem>()>>>(cfg->node_count());
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edges(node_id);
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t dest_node = edge_it->first;
      function<shared_ptr<lattice_elem>()> transfer_f = [=]() {
        return state[node_id];
      };
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        v._([&](assign *i) {
//          cout << node_id << ": " << *i << endl;
          copy_visitor cv;
          i->get_lhs()->get_id()->accept(cv);
          shared_ptr<id> id_ptr(cv.get_id());
          transfer_f = [=]() {
//            copy_visitor cv;
//            id_ptr->accept(cv);
            return shared_ptr<lattice_elem>(state[node_id]->add(definitions_t {make_tuple(dest_node, shared_ptr<id>(id_ptr))}));
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
    vector<function<shared_ptr<::analysis::reaching_defs::lattice_elem>()>> i_inc = incoming[i];
    auto constraint = [=]() {
      shared_ptr<lattice_elem> elem = this->state[i];
      for(auto transfer_f : i_inc) {
        auto calc = transfer_f();
        elem = shared_ptr<lattice_elem>(calc->lub(elem.get()));
      }
      return elem;
    };
    constraints.push_back(constraint);
  }


  _dependants = std::vector<std::set<size_t>>(cfg->node_count());
  for(auto node : *cfg) {
    auto &edges = *cfg->out_edges(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      _dependants[node->get_id()].insert(edge_it->first);
    }
  }
}

analysis::reaching_defs::reaching_defs::~reaching_defs() {
//  for(auto elem : state)
//    delete elem;
}

shared_ptr<analysis::lattice_elem> analysis::reaching_defs::reaching_defs::bottom() {
    return shared_ptr<lattice_elem>(new ::analysis::reaching_defs::lattice_elem(definitions_t{}));
}

shared_ptr<analysis::lattice_elem> analysis::reaching_defs::reaching_defs::eval(size_t node) {
  return constraints[node]();
}

std::set<size_t> analysis::reaching_defs::reaching_defs::initial() {
  set<size_t> nodes;
  for(size_t i = 0; i < cfg->node_count(); i++)
    nodes.insert(i);
  return nodes;
}

shared_ptr<::analysis::lattice_elem> analysis::reaching_defs::reaching_defs::get(size_t node) {
  return state[node];
}

void analysis::reaching_defs::reaching_defs::update(size_t node, shared_ptr<::analysis::lattice_elem> state) {
  this->state[node] = dynamic_pointer_cast<lattice_elem>(state);
}

std::set<size_t> analysis::reaching_defs::reaching_defs::dependants(size_t node_id) {
  return _dependants[node_id];
}

std::ostream &analysis::reaching_defs::operator <<(std::ostream &out, reaching_defs &_this) {
  for(size_t i = 0; i < _this.state.size(); i++) {
    out << i << ": " << *_this.state[i] << endl;
  }
  return out;
}
