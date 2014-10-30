/*
 * node_visitor.h
 *
 *  Created on: Sep 22, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <functional>

namespace cfg {

class node;
class address_node;

class node_visitor {
private:
  std::function<void(node*)> node_callback = NULL;
  std::function<void(address_node*)> address_node_callback = NULL;
public:
  virtual ~node_visitor() {
  }

  virtual void visit(node *se) {
    if(node_callback != NULL) node_callback(se);
    _default();
  }

  virtual void visit(address_node *se) {
    if(address_node_callback != NULL) address_node_callback(se);
    _default();
  }

  virtual void _default() {
  }

  void _(std::function<void(node*)> node_callback) {
    this->node_callback = node_callback;
  }

  void _(std::function<void(address_node*)> address_node_callback) {
    this->address_node_callback = address_node_callback;
  }
};

}
