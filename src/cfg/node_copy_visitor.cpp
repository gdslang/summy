/*
 * node_copy_visitor.cpp
 *
 *  Created on: Oct 29, 2014
 *      Author: Julian Kranz
 */

#include <summy/cfg/node_copy_visitor.h>
#include <summy/cfg/node.h>
#include <summy/cfg/address_node.h>

size_t cfg::node_copy_visitor::node_id(node *n) {
  if(node_id_ctor != NULL)
    return node_id_ctor(n->get_id());
  return n->get_id();
}

void cfg::node_copy_visitor::visit(node *n) {
  if(node_ctor != NULL) _node = node_ctor(node_id(n));
  else _node = new node(node_id(n));
}

void cfg::node_copy_visitor::visit(address_node *n) {
  if(address_node_ctor != NULL) _node = address_node_ctor(node_id(n), n->get_address());
  else _node = new address_node(node_id(n), n->get_address());
}
