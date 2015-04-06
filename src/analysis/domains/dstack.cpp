/*
 * dstack.cpp
 *
 *  Created on: Mar 17, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/dstack.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <cppgdsl/rreil/statement/statement.h>
#include <summy/analysis/domains/numeric/als_state.h>
#include <summy/analysis/domains/numeric/equality_state.h>
#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <functional>

using cfg::cond_edge;
using cfg::edge_visitor;
using cfg::stmt_edge;

using namespace analysis;
using namespace std;
using namespace gdsl::rreil;
using namespace analysis::value_sets;

void analysis::dstack::add_constraint(size_t from, size_t to, const ::cfg::edge *e) {
  function<shared_ptr<memory_state>()> transfer_f = [=]() {
    return state[from];
  };
  auto for_mutable = [&](function<void(shared_ptr<memory_state>)> cb) {
    transfer_f = [=]() {
      shared_ptr<memory_state> &state_c = this->state[from];
      shared_ptr<memory_state> state_new = shared_ptr<memory_state>(state_c->copy());
      cb(state_new);
      return state_new;
    };
  };
  auto for_update = [&](auto *update) {
    for_mutable([=](shared_ptr<memory_state> state_new) {
      state_new->update(update);
    });
  };
  edge_visitor ev;
  ev._([&](const stmt_edge *edge) {
    statement *stmt = edge->get_stmt();
    statement_visitor v;
    v._([&](assign *a) {
      for_update(a);
    });
    v._([&](load *l) {
      for_update(l);
    });
    v._([&](store *s) {
      for_update(s);
    });
    stmt->accept(v);
  });
  ev._([&](const cond_edge *edge) {
    for_mutable([=](shared_ptr<memory_state> state_new) {
      if(edge->is_positive())
        state_new->assume(edge->get_cond());
      else
        state_new->assume_not(edge->get_cond());
    });
  });
  e->accept(ev);
  (constraints[to])[from] = transfer_f;
}

void analysis::dstack::remove_constraint(size_t from, size_t to) {
  constraints[to].erase(from);
}

dependency analysis::dstack::gen_dependency(size_t from, size_t to) {
  return dependency { from, to };
}

void analysis::dstack::init_state() {
  size_t old_size = state.size();
  state.resize(cfg->node_count());
  for(size_t i = old_size; i < cfg->node_count(); i++) {
    if(fixpoint_pending.find(i) != fixpoint_pending.end()) state[i] = dynamic_pointer_cast<memory_state>(start_value());
    else state[i] = dynamic_pointer_cast<memory_state>(bottom());
  }
}

analysis::dstack::dstack(cfg::cfg *cfg) :
    fp_analysis(cfg) {
  init();
}

analysis::dstack::~dstack() {
}

shared_ptr<domain_state> analysis::dstack::bottom() {
//  return shared_ptr<domain_state>(memory_state::bottom(new equality_state(new als_state(vsd_state::bottom()))));
  return shared_ptr<domain_state>(memory_state::bottom(new als_state(vsd_state::bottom())));
}

std::shared_ptr<domain_state> analysis::dstack::start_value() {
//  return shared_ptr<domain_state>(memory_state::start_value(new equality_state(new als_state(vsd_state::top()))));
  return shared_ptr<domain_state>(memory_state::start_value(new als_state(vsd_state::top())));
}

shared_ptr<domain_state> analysis::dstack::get(size_t node) {
  return state[node];
}

void analysis::dstack::update(size_t node, shared_ptr<domain_state> state) {
  this->state[node] = dynamic_pointer_cast<memory_state>(state);
}

dstack_result analysis::dstack::result() {
  return dstack_result(state);
}

void analysis::dstack::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++)
    out << "Node " << i << ": " << endl << *state[i] << endl;
}
