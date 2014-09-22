/*
 * trivial_connector.cpp
 *
 *  Created on: Sep 22, 2014
 *      Author: Julian Kranz
 */

#include <summy/transformers/trivial_connector.h>
#include <summy/cfg/node_visitor.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/node.h>
#include <summy/cfg/start_node.h>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;

trivial_connector::start_node_map_t trivial_connector::start_node_map() {
  trivial_connector::start_node_map_t start_node_map;
  for(auto node : *cfg) {
//    size_t id = node->get_id();
    node_visitor nv;
    nv._([&](start_node *sn) {
      start_node_map[sn->get_address()] = sn->get_id();
    });
    node->accept(nv);
  }
  return start_node_map;
}

void trivial_connector::transform() {

}
