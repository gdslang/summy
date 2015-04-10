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

using namespace cfg;
using namespace std;

jd_manager::jd_manager(::cfg::cfg *cfg) : cfg(cfg) {
  for(auto node : *cfg) {
    node_visitor nv;
    nv._([&](address_node *an) {
      address_map[an->get_id()] = an->get_address();
    });
    node->accept(nv);

    auto &edges = *cfg->out_edge_payloads(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      nv._((function<void(class node*)>)([&](class node *n) {
        address_map[n->get_id()] = address_map[node->get_id()];
      }));
      cfg->get_node_payload(edge_it->first)->accept(nv);
    }
  }
}

jump_dir jd_manager::jump_direction(size_t from, size_t to) {
  auto from_it = address_map.find(from);
  auto to_it = address_map.find(to);
  if(from_it == address_map.end() || to_it == address_map.end())
    return UNKNOWN;
  if(to_it->second < from_it->second)
    return BACKWARD;
  else
    return FORWARD;
}
