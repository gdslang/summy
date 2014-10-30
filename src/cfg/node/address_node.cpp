/*
 * start_node.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: jucs
 */

#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node_visitor.h>

void cfg::address_node::dot(std::ostream &stream) {
  stream << get_id() << " [label=\"" << get_id() << "~" << address << "\", shape=box";
  if(!decoded)
    stream << ", color=red";
  stream << "];";
}

void cfg::address_node::accept(node_visitor &v) {
  v.visit(this);
}
