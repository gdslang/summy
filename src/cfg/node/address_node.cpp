/*
 * start_node.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: jucs
 */

#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node_visitor.h>
#include <ios>

using std::dec;
using std::hex;

void cfg::address_node::dot(std::ostream &stream) {
  stream << get_id() << " [label=\"" << get_id() << " ~ ";
  if(name)
    stream << name.value() << ":";
  stream << "0x" << hex << address << dec << "\", shape=box";
  switch(decs) {
    case DECODED: {
      break;
    }
    case DECODABLE: {
      stream << ", color=green";
      break;
    }
    case UNDEFINED: {
      stream << ", color=red";
      break;
    }
  }
  stream << "];";
}

void cfg::address_node::accept(node_visitor &v) {
  v.visit(this);
}
