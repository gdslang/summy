/*
 * node.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: jucs
 */

#include <summy/cfg/node.h>
#include <summy/cfg/node_visitor.h>

void cfg::node::dot(std::ostream &stream) {
  stream << id << ";";
}

void cfg::node::accept(node_visitor &v) {
  v.visit(this);
}
