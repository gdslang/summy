/*
 * addr.cpp
 *
 *  Created on: Dec 23, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/addr/addr.h>
#include <summy/analysis/addr/addr_state.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node.h>
#include <summy/cfg/node/node_visitor.h>
#include <functional>
#include <experimental/optional>

using namespace std;
using namespace analysis;
using namespace analysis::addr;
using namespace std::experimental;

void analysis::addr::addr::add_constraint(size_t from, size_t to, const ::cfg::edge *e) {
  function<shared_ptr<addr_state>()> transfer_f = [=]() {
    cfg::node *to_node = cfg->get_node_payload(to);
    cfg::node_visitor nv;
    optional<size_t> address;
    nv._([&](cfg::address_node *cn) { address = cn->get_address(); });
    to_node->accept(nv);
    if(address)
      return make_shared<addr_state>(address.value());
    else
      return state[from];
  };
  (constraints[to])[from] = transfer_f;
}

void analysis::addr::addr::remove_constraint(size_t from, size_t to) {
  (constraints[to]).erase(from);
}

dependency analysis::addr::addr::gen_dependency(size_t from, size_t to) {
  return dependency{from, to};
}

void analysis::addr::addr::init_state() {
//  cout << "Resize: " << cfg->node_count() << endl;
  size_t old_size = state.size();
  state.resize(cfg->node_count());
  for(size_t i = old_size; i < cfg->node_count(); i++) {
    if(fixpoint_pending.find(i) != fixpoint_pending.end())
      state[i] = start_value(i);
    else
      state[i] = dynamic_pointer_cast<addr_state>(bottom());
  }
}

analysis::addr::addr::addr(cfg::cfg *cfg) : fp_analysis(cfg) {
  init();
}

analysis::addr::addr::~addr() {
}

std::shared_ptr<addr_state> analysis::addr::addr::bottom() {
  return make_shared<addr_state>();
}

std::shared_ptr<addr_state> analysis::addr::addr::start_value(size_t node) {
  cfg::node *node_pl = cfg->get_node_payload(node);
  cfg::node_visitor nv;
  optional<size_t> address;
  nv._([&](cfg::address_node *cn) { address = cn->get_address(); });
  node_pl->accept(nv);
  return make_shared<addr_state>(address.value());
}

std::shared_ptr<domain_state> analysis::addr::addr::get(size_t node) {
  return state[node];
}

void analysis::addr::addr::update(size_t node, std::shared_ptr<domain_state> state) {
  this->state[node] = dynamic_pointer_cast<addr_state>(state);
}

addr_result analysis::addr::addr::result() {
  return addr_result(state);
}

void analysis::addr::addr::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++) {
    if(i != 0)
      cout << endl;
    out << i << ": " << *state[i];
  }
}
