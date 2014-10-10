
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

using namespace std;
using namespace cfg;
using namespace analysis::liveness;
using namespace gdsl::rreil;

void analysis::liveness::liveness::init_constraints() {
  constraints = vector<constraint_t>(cfg->node_count());
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    vector<constraint_t> node_constraints;
    auto &edges = *cfg->out_edges(node_id);
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t dest_node = edge_it->first;
      constraint_t transfer_f = [=]() {
        return state[node_id];
      };
      edge_visitor ev;
      ev._([&](stmt_edge *edge) { //Todo: cond_edge
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        auto id_assigned = [&](id *i) {
//          copy_visitor cv;
//          i->accept(cv);
//          shared_ptr<id> id_ptr(cv.get_id());
//          transfer_f = [=]() {
//            auto defs_rm = shared_ptr<rd_elem>(state[node_id]->remove(id_set_t { id_ptr }));
//            return shared_ptr<rd_elem>(defs_rm->add(definitions_t {make_tuple(dest_node, id_ptr)}));
//          };
        };
        v._([&](assign *i) {
          lv_elem::elements_t newly_live;
          visitor id_acc;
          id_acc._((function<void(variable*)>)([&](variable *var) {
            copy_visitor cv;
            var->get_id()->accept(cv);
            newly_live.insert(shared_ptr<id>(cv.get_id()));
          }));
          i->accept(id_acc);
          transfer_f = [=]() {
            return shared_ptr<lv_elem>(state[dest_node]->add(newly_live));
          };
        });
        v._([&](load *l) {
          id_assigned(l->get_lhs()->get_id());
        });
        stmt->accept(v);
      });
      edge_it->second->accept(ev);
      auto constraint = [=]() {
        shared_ptr<lv_elem> elem = this->state[node_id];
        for(auto transfer_f : node_constraints) {
          auto calc = transfer_f();
          elem = shared_ptr<lv_elem>(calc->lub(elem.get()));
        }
        return elem;
      };
      constraints[node_id] = constraint;
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

shared_ptr<analysis::lattice_elem> analysis::liveness::liveness::eval(size_t node) {
  return constraints[node]();
}

std::set<size_t> analysis::liveness::liveness::initial() {
  set<size_t> nodes;
  for(size_t i = 0; i < cfg->node_count(); i++)
    nodes.insert(i);
  return nodes;
}

shared_ptr<analysis::lattice_elem> analysis::liveness::liveness::get(size_t node) {
}

void analysis::liveness::liveness::update(size_t node, shared_ptr<lattice_elem> state) {
}

std::set<size_t> analysis::liveness::liveness::dependants(size_t node_id) {
}

std::ostream& analysis::liveness::operator <<(std::ostream& out, liveness& _this) {
}
