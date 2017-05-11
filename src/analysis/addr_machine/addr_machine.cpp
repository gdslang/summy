/*
 * addr.cpp
 *
 *  Created on: Dec 23, 2015
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <experimental/optional>
#include <functional>
#include <summy/analysis/addr_machine/addr_machine.h>
#include <summy/analysis/addr_machine/addr_machine_state.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node.h>
#include <summy/cfg/node/node_visitor.h>

using cfg::edge_visitor;
using cfg::stmt_edge;
using cfg::cond_edge;

using namespace std;
using namespace analysis;
using namespace analysis::addr_machine;
using namespace std::experimental;

void analysis::addr_machine::addr_machine::add_constraint(
  size_t from, size_t to, const ::cfg::edge *) {
  constraint_t transfer_f = [=](size_t) {
    cfg::node *to_node = cfg->get_node_payload(to);
    cfg::node_visitor nv;
    optional<size_t> address;
    nv._([&](cfg::address_node *cn) { address = cn->get_address(); });
    to_node->accept(nv);
    if(address)
      return default_context(make_shared<addr_machine_state>(address.value()));
    else
      return default_context(state[from]);
  };
  (constraints[to])[from] = transfer_f;
}

void analysis::addr_machine::addr_machine::remove_constraint(size_t from, size_t to) {
  (constraints[to]).erase(from);
}

dependency analysis::addr_machine::addr_machine::gen_dependency(size_t from, size_t to) {
  return dependency{from, to};
}

void analysis::addr_machine::addr_machine::init_state() {
  //  cout << "Resize: " << cfg->node_count() << endl;
  size_t old_size = state.size();
  state.resize(cfg->node_count());
  for(size_t i = old_size; i < cfg->node_count(); i++) {
    if(fixpoint_pending.find(i) != fixpoint_pending.end())
      state[i] = start_value(i);
    else
      state[i] = dynamic_pointer_cast<addr_machine_state>(bottom());
  }
}

analysis::addr_machine::addr_machine::addr_machine(cfg::cfg *cfg) : fp_analysis(cfg) {
  init();
}

analysis::addr_machine::addr_machine::~addr_machine() {}

std::shared_ptr<addr_machine_state> analysis::addr_machine::addr_machine::bottom() {
  return make_shared<addr_machine_state>();
}

std::shared_ptr<addr_machine_state> analysis::addr_machine::addr_machine::start_value(size_t node) {
  cfg::node *node_pl = cfg->get_node_payload(node);
  cfg::node_visitor nv;
  optional<size_t> address;
  nv._([&](cfg::address_node *cn) { address = cn->get_address(); });
  node_pl->accept(nv);
  if(address)
    return make_shared<addr_machine_state>(address.value());
  else
    return make_shared<addr_machine_state>();
}

std::shared_ptr<domain_state> analysis::addr_machine::addr_machine::get(size_t node) {
  return state[node];
}

void analysis::addr_machine::addr_machine::update(
  analysis_node node, std::shared_ptr<domain_state> state) {
  this->state[node.id] = dynamic_pointer_cast<addr_machine_state>(state);
}

addr_machine_result analysis::addr_machine::addr_machine::result() {
  return addr_machine_result(state);
}

void analysis::addr_machine::addr_machine::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++) {
    if(i != 0) cout << endl;
    out << i << ": " << *state[i];
  }
}
