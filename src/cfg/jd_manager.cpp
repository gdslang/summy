/*
 * jd_manager.cpp
 *
 *  Created on: Apr 10, 2015
 *      Author: Julian Kranz
 */


#include <summy/cfg/jd_manager.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node.h>
#include <summy/cfg/node/node_visitor.h>
#include <functional>
#include <map>
#include <set>
#include <assert.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>

using namespace cfg;
using namespace std;

jd_manager::jd_manager(::cfg::cfg *cfg) : addr(cfg), fp(&addr, *this, false), cfg(cfg) {
  notified = false;
  fp.iterate();
  cfg->register_observer(&fp);
  cfg->register_observer(this);
}

jump_dir jd_manager::jump_direction(size_t from, size_t to) {
  if(notified) {
    notified = false;
    fp.iterate();
  }

  bool is_call_edge = false;
  edge_visitor ev;
  ev._([&](call_edge const *ce) {
    is_call_edge = true;
  });
  auto edge_it = cfg->out_edge_payloads(from)->find(to);
  assert(edge_it != cfg->out_edge_payloads(from)->end());
  edge_it->second->accept(ev);
  if(is_call_edge)
    return UNKNOWN;

  if(edge_it->second->get_jump_dir() == BACKWARD)
    return BACKWARD;

  analysis::addr_machine::addr_machine_result ar = addr.result();
  auto from_address = ar.result.at(from)->get_address();
  auto to_address = ar.result.at(to)->get_address();

  /*
   * There should be an address for every node?! => Assertions
   * => No, because the 'address' analysis also uses the
   * fixpoint engine
   */
//  assert(from_address);
//  assert(to_address);

//  cout << (from_address ? from_address.value() : 0) << " / " << (to_address ? to_address.value() : 0)  << endl;
  if(!from_address || !to_address)
    return UNKNOWN;
  if(to_address.value() < from_address.value())
    return BACKWARD;
  else
    return FORWARD;
}

size_t jd_manager::machine_address_of(size_t node) {
  if(notified) {
    notified = false;
    fp.iterate();
  }
  analysis::addr_machine::addr_machine_result ar = addr.result();
  auto node_addr =  ar.result.at(node)->get_address();
  if(node_addr)
    return node_addr.value();
  else
    return 0;
}

std::shared_ptr<analysis::addr_machine::addr_machine_state> jd_manager::address_of(size_t node) {
  if(notified) {
    notified = false;
    fp.iterate();
  }
  analysis::addr_machine::addr_machine_result ar = addr.result();
  auto node_state =  ar.result.at(node);
  return node_state;
}

void jd_manager::notify(const std::vector<update> &updates) {
  addr.analysis::fp_analysis::update(updates);
  notified = true;
}
