/*
 * node.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: jucs
 */

#include <summy/cfg/node/node.h>
#include <summy/cfg/node/node_visitor.h>

void cfg::node::put(std::ostream &out) {
  out << "Node(" << id << ")";
}

void cfg::node::dot(std::ostream &stream) {
  stream << id << ";";
}

void cfg::node::accept(node_visitor &v) {
  v.visit(this);
}

std::ostream& cfg::operator <<(std::ostream& out, node& _this) {
  _this.put(out);
  return out;
}
