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

  cfg->register_observer(this);
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

void jd_manager::notify(const vector<update> &updates) {
  /*
   * Todo: We need a way to bfs on the updated part of the graph...
   */
//  map<size_t, set<size_t>> update_graph;
//  node_visitor nv;
//
//  function<void(size_t)> process_graph_from;
//  process_graph_from = [&](size_t id) {
//    auto dest_current_it = update_graph.find(id);
//    set<size_t> dest_current = dest_current_it->second;
//    update_graph.erase(dest_current_it);
//    for(auto &dest : dest_current) {
//      nv._([&](address_node *an) {
//        address_map[an->get_id()] = an->get_address();
//      });
//      nv._((function<void(node*)>)([&](node *n) {
//        address_map[n->get_id()] = address_map[id];
//      }));
//      cfg->get_node_payload(dest)->accept(nv);
//    }
//    for(auto &dest : dest_current)
//      process_graph_from(dest);
//  };
//
//  for(auto &update : updates) {
//    update_graph[update.from].insert(update.to);
//
//    bool from_address = false;
//    nv._([&](address_node *an) {
//      address_map[an->get_id()] = an->get_address();
//      from_address = true;
//    });
//    nv._((function<void(node*)>)([&](node *n) {
//      auto addr_map_it = address_map.find(n->get_id());
//      if(addr_map_it != address_map.end())
//        from_address = true;
//    }));
//    cfg->get_node_payload(update.from)->accept(nv);
//
//    if(from_address)
//      process_graph_from(update.from);
//  }
}
