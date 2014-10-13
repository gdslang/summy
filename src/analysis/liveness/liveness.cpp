
/*
 * liveness.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/liveness/liveness.h>
#include <summy/analysis/lattice_elem.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/edge.h>
#include <cppgdsl/rreil/rreil.h>
#include <cppgdsl/rreil/statement/statement_visitor.h>
#include <cppgdsl/rreil/copy_visitor.h>
#include <functional>
#include <memory>
#include <vector>
#include <set>
#include <iostream>

using namespace std;
using namespace cfg;
using namespace analysis::liveness;
using namespace gdsl::rreil;

void analysis::liveness::liveness::init_constraints() {
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    vector<constraint_t> node_constraints;
    auto &edges = *cfg->out_edges(node_id);
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t dest_node = edge_it->first;
      constraint_t transfer_f = [=]() {
        return state[dest_node];
      };
      edge_visitor ev;
      ev._([&](stmt_edge *edge) { //Todo: cond_edge
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        auto get_newly_live = [&](function<void(visitor&)> accept_live) {
          copy_visitor cv;
          lv_elem::elements_t newly_live;
          visitor id_acc;
          id_acc._((function<void(variable*)>)([&](variable *var) {
            var->get_id()->accept(cv);
            newly_live.insert(singleton_t(cv.get_id()));
          }));
          accept_live(id_acc);
          return newly_live;
        };
        auto assignment = [&](function<void(visitor&)> accept_live, function<void(visitor&)> accept_dead) {
          auto newly_live = get_newly_live(accept_live);

          copy_visitor cv;
          accept_dead(cv);
          singleton_t lhs(cv.get_id());

          transfer_f = [=]() {
            cout << *lhs << endl;
            shared_ptr<lv_elem> dead_removed(state[dest_node]->remove({ lhs }));
            cout << "after dead_removed: " << *dead_removed << endl;
            return shared_ptr<lv_elem>(dead_removed->add(newly_live));
          };
        };
        auto access = [&](function<void(visitor&)> accept_live) {
          auto newly_live = get_newly_live(accept_live);
          transfer_f = [=]() {
            return shared_ptr<lv_elem>(state[dest_node]->add(newly_live));
          };
        };
        v._([&](assign *i) {
          assignment([&](visitor &v) {
            i->get_rhs()->accept(v);
          },
          [&](visitor &v) {
            i->get_lhs()->get_id()->accept(v);
          });
        });
        v._([&](load *l) {
          assignment([&](visitor &v) {
            l->get_address()->accept(v);
          },
          [&](visitor &v) {
            l->get_lhs()->get_id()->accept(v);
          });
        });
        v._default([&](statement *s) {
          access([&](visitor &v){
            s->accept(v);
          });
        });
        stmt->accept(v);
      });
      edge_it->second->accept(ev);
      constraints[node_id].push_back(transfer_f);
    }
  }
}

void analysis::liveness::liveness::init_dependants() {
  _dependants = vector<set<size_t>>(cfg->node_count());
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edges(node_id);
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++)
      _dependants[edge_it->first].insert(node_id);
  }
}

analysis::liveness::liveness::liveness(class cfg *cfg) : analysis(cfg) {
  state = state_t(cfg->node_count());
  for(size_t i = 0; i < state.size(); i++)
    state[i] = dynamic_pointer_cast<lv_elem>(bottom());

  init_constraints();
  init_dependants();
}

analysis::liveness::liveness::~liveness() {
}

shared_ptr<analysis::lattice_elem> analysis::liveness::liveness::bottom() {
  return make_shared<lv_elem>(lv_elem::elements_t {});
}

std::set<size_t> analysis::liveness::liveness::initial() {
  set<size_t> nodes;
//  for(size_t i = 0; i < cfg->node_count(); i++)
//    nodes.insert(i);
  nodes.insert(49);
  return nodes;
}

shared_ptr<analysis::lattice_elem> analysis::liveness::liveness::get(size_t node) {
  return state[node];
}

void analysis::liveness::liveness::update(size_t node, shared_ptr<lattice_elem> state) {
  this->state[node] = dynamic_pointer_cast<lv_elem>(state);
}

std::set<size_t> analysis::liveness::liveness::dependants(size_t node_id) {
  return _dependants[node_id];
}

std::ostream& analysis::liveness::operator <<(std::ostream &out, liveness &_this) {
  for(size_t i = 0; i < _this.state.size(); i++) {
    out << i << ": " << *_this.state[i] << endl;
  }
  return out;
}
