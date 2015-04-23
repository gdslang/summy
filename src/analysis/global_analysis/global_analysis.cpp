/*
 * global_analysis.cpp
 *
 *  Created on: Apr 23, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/global_analysis/global_analysis.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <cppgdsl/rreil/statement/statement.h>
#include <summy/analysis/domains/numeric/als_state.h>
#include <summy/analysis/domains/numeric/equality_state.h>
#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/analysis/static_memory.h>
#include <summy/value_set/vs_finite.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <functional>

using cfg::cond_edge;
using cfg::edge_visitor;
using cfg::stmt_edge;

using namespace analysis;
using namespace std;
using namespace gdsl::rreil;

void analysis::global_analysis::add_constraint(size_t from, size_t to, const ::cfg::edge *e) {
  /*
   * Hier genau für call-edge von caller-Knoten zu return-Knoten/consecutive
   *
   * Callee wird dep. von caller, sodass fp ihn auswertet
   * Callee muss auf start_value initialisiert werden, falls aktuell bottom
   */

//  function<shared_ptr<memory_state>()> transfer_f = [=]() {
//    return state[from];
//  };
//  auto for_mutable = [&](function<void(shared_ptr<memory_state>)> cb) {
//    transfer_f = [=]() {
//      shared_ptr<memory_state> &state_c = this->state[from];
//      shared_ptr<memory_state> state_new = shared_ptr<memory_state>(state_c->copy());
//      cb(state_new);
//      return state_new;
//    };
//  };
//  auto for_update = [&](auto *update) {
//    for_mutable([=](shared_ptr<memory_state> state_new) {
//      state_new->update(update);
//    });
//  };
//  edge_visitor ev;
//  ev._([&](const stmt_edge *edge) {
//    statement *stmt = edge->get_stmt();
//    statement_visitor v;
//    v._([&](assign *a) {
//      for_update(a);
//    });
//    v._([&](load *l) {
//      for_update(l);
//    });
//    v._([&](store *s) {
//      for_update(s);
//    });
//    stmt->accept(v);
//  });
//  ev._([&](const cond_edge *edge) {
//    for_mutable([=](shared_ptr<memory_state> state_new) {
//      if(edge->is_positive())
//        state_new->assume(edge->get_cond());
//      else
//        state_new->assume_not(edge->get_cond());
//    });
//  });
//  e->accept(ev);
//  (constraints[to])[from] = transfer_f;
}

void analysis::global_analysis::remove_constraint(size_t from, size_t to) {
//  constraints[to].erase(from);
}

dependency analysis::global_analysis::gen_dependency(size_t from, size_t to) {
//  return dependency { from, to };
}

void analysis::global_analysis::init_state() {
//  size_t old_size = state.size();
//  state.resize(cfg->node_count());
//  for(size_t i = old_size; i < cfg->node_count(); i++) {
//    if(fixpoint_pending.find(i) != fixpoint_pending.end()) state[i] = dynamic_pointer_cast<memory_state>(start_value());
//    else state[i] = dynamic_pointer_cast<memory_state>(bottom());
//  }
}

analysis::global_analysis::global_analysis(cfg::cfg *cfg, std::shared_ptr<static_memory> sm) :
  fp_analysis(cfg), sm(sm) {
  init();
}

analysis::global_analysis::global_analysis(cfg::cfg *cfg) :
    global_analysis(cfg, make_shared<static_dummy>()) {
  init();
}

analysis::global_analysis::~global_analysis() {
}

shared_ptr<domain_state> analysis::global_analysis::bottom() {
//  return shared_ptr<domain_state>(memory_state::bottom(sm, new equality_state(new als_state(vsd_state::bottom(sm)))));
//  return shared_ptr<domain_state>(memory_state::bottom(new als_state(vsd_state::bottom())));
}

std::shared_ptr<domain_state> analysis::global_analysis::start_value() {
//  return shared_ptr<domain_state>(memory_state::start_value(sm, new equality_state(new als_state(vsd_state::top(sm)))));
//  return shared_ptr<domain_state>(memory_state::start_value(new als_state(vsd_state::top())));
}

shared_ptr<domain_state> analysis::global_analysis::get(size_t node) {
//  return state[node];
}

void analysis::global_analysis::update(size_t node, shared_ptr<domain_state> state) {
//  this->state[node] = dynamic_pointer_cast<memory_state>(state);
}

global_analysis_result analysis::global_analysis::result() {
//  return global_analysis_result(state);
}

void analysis::global_analysis::put(std::ostream &out) {
//  for(size_t i = 0; i < state.size(); i++)
//    out << "Node " << i << ": " << endl << *state[i] << endl;
}
