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

void reaching_defs::init_constraints() {
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edges(node_id);
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t dest_node = edge_it->first;
      function<shared_ptr<rd_elem>()> transfer_f = [=]() {
        return state[node_id];
      };
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        auto id_assigned = [&](id *i) {
          copy_visitor cv;
          i->accept(cv);
          shared_ptr<id> id_ptr(cv.get_id());
          transfer_f = [=]() {
            auto defs_rm = shared_ptr<rd_elem>(state[node_id]->remove(id_set_t { id_ptr }));
            return shared_ptr<rd_elem>(defs_rm->add(rd_elem::elements_t {make_tuple(dest_node, id_ptr)}));
          };
        };
        v._([&](assign *i) {
          id_assigned(i->get_lhs()->get_id());
        });
        v._([&](load *l) {
          id_assigned(l->get_lhs()->get_id());
        });
        stmt->accept(v);
      });
      edge_it->second->accept(ev);
      constraints[dest_node].push_back(transfer_f);
    }
  }

  assert(constraints.size() == cfg->node_count());
}

void reaching_defs::init_dependants() {
  _dependants = std::vector<std::set<size_t>>(cfg->node_count());
  for(auto node : *cfg) {
    auto &edges = *cfg->out_edges(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      _dependants[node->get_id()].insert(edge_it->first);
    }
  }
}

void reaching_defs::init_fixpoint_initial() {
  for(size_t i = 0; i < cfg->node_count(); i++)
    fixpoint_initial.insert(i);

  for(auto deps : _dependants)
    for(auto dep : deps) {
      fixpoint_initial.erase(dep);
    }
}

reaching_defs::reaching_defs::reaching_defs(class cfg *cfg) : analysis::analysis(cfg) {
  init_constraints();
  init_dependants();
  init_fixpoint_initial();

  state = state_t(cfg->node_count());
  for(size_t i = 0; i < state.size(); i++)
    if(fixpoint_initial.find(i) != fixpoint_initial.end())
      state[i] = dynamic_pointer_cast<rd_elem>(start_value());
    else
      state[i] = dynamic_pointer_cast<rd_elem>(bottom());
}

analysis::reaching_defs::reaching_defs::~reaching_defs() {
}

shared_ptr<analysis::lattice_elem> reaching_defs::reaching_defs::bottom() {
    return shared_ptr<rd_elem>(new rd_elem());
}

shared_ptr<analysis::lattice_elem> reaching_defs::reaching_defs::start_value() {
    return shared_ptr<rd_elem>(new rd_elem(rd_elem::elements_t {}));
}

std::set<size_t> analysis::reaching_defs::reaching_defs::initial() {
  return fixpoint_initial;
}

shared_ptr<::analysis::lattice_elem> reaching_defs::reaching_defs::get(size_t node) {
  return state[node];
}

void reaching_defs::update(size_t node, shared_ptr<::analysis::lattice_elem> state) {
  this->state[node] = dynamic_pointer_cast<rd_elem>(state);
}

std::set<size_t> reaching_defs::dependants(size_t node_id) {
  return _dependants[node_id];
}

std::ostream &analysis::reaching_defs::operator <<(std::ostream &out, reaching_defs &_this) {
  for(size_t i = 0; i < _this.state.size(); i++) {
    out << i << ": " << *_this.state[i] << endl;
  }
  return out;
}
