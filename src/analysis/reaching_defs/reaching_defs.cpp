/*
 * reaching_defs.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <cppgdsl/rreil/copy_visitor.h>
#include <cppgdsl/rreil/rreil.h>
#include <functional>
#include <memory>
#include <set>
#include <summy/analysis/domain_state.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/analysis/reaching_defs/reaching_defs.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/tools/rreil_util.h>
#include <tuple>
#include <vector>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;
using namespace analysis::reaching_defs;
using analysis::liveness::liveness_result;

std::map<size_t, std::shared_ptr<::analysis::domain_state>>
analysis::reaching_defs::reaching_defs::transform(
  size_t from, size_t to, const ::cfg::edge *e, size_t from_ctx) {
  auto cleanup_live = [=](shared_ptr<rd_state> acc) {
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
    return shared_ptr<rd_state>(acc->remove(
      [&](size_t def, shared_ptr<id> id) { return !lv_result.contains(to, id, 0, 64); }));
  };
  constraint_t transfer_f = [=](size_t) { return default_context(cleanup_live(state[from])); };
  edge_visitor ev;
  ev._([&](const stmt_edge *edge) {
    statement *stmt = edge->get_stmt();
    statement_visitor v;
    auto id_assigned = [&](int_t size, variable const *v) {
      copy_visitor cv;
      v->get_id().accept(cv);
      shared_ptr<id> id_ptr(cv.retrieve_id());
      transfer_f = [=](size_t) {
        auto acc = state[from];
        if(lv_result.contains(to, id_ptr, v->get_offset(), size)) {
          acc = shared_ptr<rd_state>(acc->remove(id_set_t{id_ptr}));
          acc = shared_ptr<rd_state>(acc->add(rd_state::elements_t{make_tuple(to, id_ptr)}));
        }
        return default_context(cleanup_live(acc));
      };
    };
    v._([&](assign const *a) { id_assigned(a->get_size(), &a->get_lhs()); });
    v._([&](load const *l) { id_assigned(l->get_size(), &l->get_lhs()); });
    stmt->accept(v);
  });
  e->accept(ev);

  return transfer_f(from_ctx);
}

analysis::dependency analysis::reaching_defs::reaching_defs::gen_dependency(
  size_t from, size_t to) {
  return dependency{from, to};
}

void analysis::reaching_defs::reaching_defs::init_state() {
  size_t old_size = state.size();
  state.resize(cfg->node_count());
  for(size_t i = old_size; i < cfg->node_count(); i++)
    state[i] = dynamic_pointer_cast<rd_state>(bottom());
}

reaching_defs::reaching_defs::reaching_defs(class cfg *cfg, liveness_result lv_result)
    : fp_analysis::fp_analysis(cfg, analysis_direction::FORWARD), lv_result(lv_result) {
  init();
}

analysis::reaching_defs::reaching_defs::~reaching_defs() {}

shared_ptr<analysis::domain_state> reaching_defs::reaching_defs::bottom() {
  return shared_ptr<rd_state>(new rd_state());
}

shared_ptr<analysis::domain_state> reaching_defs::reaching_defs::start_state(size_t) {
  return shared_ptr<rd_state>(new rd_state(rd_state::elements_t{}));
}

shared_ptr<::analysis::domain_state> reaching_defs::reaching_defs::get(size_t node) {
  return state[node];
}

void reaching_defs::update(analysis_node node, shared_ptr<::analysis::domain_state> state) {
  this->state[node.id] = dynamic_pointer_cast<rd_state>(state);
}

reaching_defs_result_t analysis::reaching_defs::reaching_defs::result() {
  return reaching_defs_result_t(state);
}

void analysis::reaching_defs::reaching_defs::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++)
    out << i << ": " << *state[i] << endl;
}
