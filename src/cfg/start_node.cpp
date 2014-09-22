/*
 * start_node.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: jucs
 */

#include <summy/cfg/start_node.h>
#include <summy/cfg/node_visitor.h>

void cfg::start_node::dot(std::ostream &stream) {
  stream << get_id() << " [label=" << address << ", shape=box];";
}

void cfg::start_node::accept(node_visitor &v) {
  v.visit(this);
}
