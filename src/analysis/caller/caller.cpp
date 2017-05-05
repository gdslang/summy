/*
 * caller.cpp
 *
 *  Created on: Feb 09, 2016
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <experimental/optional>
#include <functional>
#include <summy/analysis/caller/caller.h>
#include <summy/analysis/caller/caller_state.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node.h>
#include <summy/cfg/node/node_visitor.h>

using cfg::call_edge;
using cfg::edge_visitor;
using cfg::stmt_edge;
using cfg::cond_edge;

using namespace std;
using namespace analysis;
using namespace analysis::caller;
using namespace std::experimental;

void analysis::caller::caller::add_constraint(size_t from, size_t to, const ::cfg::edge *e) {
  constraint_t transfer_f = [=](size_t) {
    bool is_call_target = false;
    edge_visitor ev;
    ev._([&](const call_edge *edge) { is_call_target = edge->is_target_edge(); });
    e->accept(ev);
    auto parent = state[from];
    if(is_call_target)
      return default_context(shared_ptr<caller_state>(parent->add_caller(from)));
    else
      return default_context(shared_ptr<caller_state>(new caller_state(*parent)));

  };
  (constraints[to])[from] = transfer_f;
}

void analysis::caller::caller::remove_constraint(size_t from, size_t to) {
  (constraints[to]).erase(from);
}

dependency analysis::caller::caller::gen_dependency(size_t from, size_t to) {
  return dependency{from, to};
}

void analysis::caller::caller::init_state() {
  //  cout << "Resize: " << cfg->node_count() << endl;
  size_t old_size = state.size();
  state.resize(cfg->node_count());
  for(size_t i = old_size; i < cfg->node_count(); i++) {
    if(fixpoint_pending.find(i) != fixpoint_pending.end())
      state[i] = start_value(i);
    else
      state[i] = dynamic_pointer_cast<caller_state>(bottom());
  }
}

analysis::caller::caller::caller(cfg::cfg *cfg) : fp_analysis(cfg) {
  init();
}

analysis::caller::caller::~caller() {}

std::shared_ptr<caller_state> analysis::caller::caller::bottom() {
  return make_shared<caller_state>();
}

std::shared_ptr<caller_state> analysis::caller::caller::start_value(size_t node) {
  return bottom();
}

std::shared_ptr<domain_state> analysis::caller::caller::get(size_t node) {
  return state[node];
}

void analysis::caller::caller::update(size_t node, std::shared_ptr<domain_state> state) {
  this->state[node] = dynamic_pointer_cast<caller_state>(state);
}

caller_result analysis::caller::caller::result() {
  return caller_result(state);
}

void analysis::caller::caller::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++) {
    if(i != 0) cout << endl;
    out << i << ": " << *state[i];
  }
}
