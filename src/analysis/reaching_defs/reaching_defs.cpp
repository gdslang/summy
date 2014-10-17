/*
 * reaching_defs.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/reaching_defs/reaching_defs.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge.h>
#include <summy/tools/rreil_util.h>
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
using analysis::liveness::liveness_result;

void reaching_defs::init_constraints() {
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edges(node_id);
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t dest_node = edge_it->first;
      auto cleanup_live = [=](shared_ptr<rd_elem> acc) {
//        id_set_t rm_by_lv;
//        cout << node_id << "->" << dest_node << ": ";
//        for(auto newly_live : lv_result.pn_newly_live[node_id])
//          if(!lv_result.contains(dest_node, newly_live)) {
//            shared_ptr<id> newly_live_id;
//            tie(newly_live_id, ignore) = newly_live;
//            cout << *newly_live_id << " ";
//            rm_by_lv.insert(newly_live_id);
//          }
//        cout << endl;
//        return shared_ptr<rd_elem>(acc->remove(rm_by_lv));
        return shared_ptr<rd_elem>(acc->remove([&](size_t def, shared_ptr<id> id) {
          return !lv_result.contains(dest_node, id, 0, 64);
        }));
      };
      function<shared_ptr<rd_elem>()> transfer_f = [=]() {
        return cleanup_live(state[node_id]);
      };
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        auto id_assigned = [&](int_t size, variable *v) {
          copy_visitor cv;
          v->get_id()->accept(cv);
          shared_ptr<id> id_ptr(cv.get_id());
         transfer_f = [=]() {
            auto acc = shared_ptr<rd_elem>(state[node_id]->remove(id_set_t { id_ptr }));
            if(lv_result.contains(dest_node, id_ptr, v->get_offset(), size))
              acc = shared_ptr<rd_elem>(acc->add(rd_elem::elements_t {make_tuple(dest_node, id_ptr)}));
            return cleanup_live(acc);
          };
        };
        v._([&](assign *a) {
          id_assigned(rreil_prop::size_of_assign(a), a->get_lhs());
        });
        v._([&](load *l) {
          id_assigned(l->get_size(), l->get_lhs());
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
  for(auto node : *cfg) {
    auto &edges = *cfg->out_edges(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      _dependants[node->get_id()].insert(edge_it->first);
    }
  }
}

reaching_defs::reaching_defs::reaching_defs(class cfg *cfg, liveness_result lv_result) :
    analysis::analysis(cfg), lv_result(lv_result) {
  init();

  state = state_t(cfg->node_count());
  for(size_t i = 0; i < state.size(); i++)
    if(fixpoint_initial.find(i) != fixpoint_initial.end()) state[i] = dynamic_pointer_cast<rd_elem>(start_value());
    else state[i] = dynamic_pointer_cast<rd_elem>(bottom());
}

analysis::reaching_defs::reaching_defs::~reaching_defs() {
}

shared_ptr<analysis::lattice_elem> reaching_defs::reaching_defs::bottom() {
    return shared_ptr<rd_elem>(new rd_elem());
}

shared_ptr<analysis::lattice_elem> reaching_defs::reaching_defs::start_value() {
    return shared_ptr<rd_elem>(new rd_elem(rd_elem::elements_t {}));
}

shared_ptr<::analysis::lattice_elem> reaching_defs::reaching_defs::get(size_t node) {
  return state[node];
}

void reaching_defs::update(size_t node, shared_ptr<::analysis::lattice_elem> state) {
  this->state[node] = dynamic_pointer_cast<rd_elem>(state);
}

reaching_defs_result_t analysis::reaching_defs::reaching_defs::result() {
  return state;
}

void analysis::reaching_defs::reaching_defs::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++)
    out << i << ": " << *state[i] << endl;
}
