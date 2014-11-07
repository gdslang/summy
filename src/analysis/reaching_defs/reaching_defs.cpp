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
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
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

void analysis::reaching_defs::reaching_defs::add_constraint(size_t from, size_t to, const ::cfg::edge* e) {
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
      return !lv_result.contains(to, id, 0, 64);
    }));
  };
  function<shared_ptr<rd_elem>()> transfer_f = [=]() {
    return cleanup_live(state[from]);
  };
  edge_visitor ev;
  ev._([&](const stmt_edge *edge) {
    statement *stmt = edge->get_stmt();
    statement_visitor v;
    auto id_assigned = [&](int_t size, variable *v) {
      copy_visitor cv;
      v->get_id()->accept(cv);
      shared_ptr<id> id_ptr(cv.get_id());
      transfer_f = [=]() {
        auto acc = shared_ptr<rd_elem>(state[from]->remove(id_set_t { id_ptr }));
        if(lv_result.contains(to, id_ptr, v->get_offset(), size))
          acc = shared_ptr<rd_elem>(acc->add(rd_elem::elements_t {make_tuple(to, id_ptr)}));
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
  e->accept(ev);
  (constraints[to])[from] = transfer_f;
}

void analysis::reaching_defs::reaching_defs::remove_constraint(size_t from, size_t to) {
  constraints[to].erase(from);
}

analysis::dependency analysis::reaching_defs::reaching_defs::gen_dependency(size_t from, size_t to) {
  return dependency { from, to };
}

void analysis::reaching_defs::reaching_defs::init_state() {
  size_t old_size = state.size();
  state.resize(cfg->node_count());
  for(size_t i = old_size; i < cfg->node_count(); i++) {
    if(fixpoint_pending.find(i) != fixpoint_pending.end()) state[i] = dynamic_pointer_cast<rd_elem>(
        start_value());
    else state[i] = dynamic_pointer_cast<rd_elem>(bottom());
  }
}

reaching_defs::reaching_defs::reaching_defs(class cfg *cfg, liveness_result lv_result) :
    fp_analysis::fp_analysis(cfg), lv_result(lv_result) {
  init();
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
  return reaching_defs_result_t(state);
}

void analysis::reaching_defs::reaching_defs::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++)
    out << i << ": " << *state[i] << endl;
}
