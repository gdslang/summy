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
class start_node;

class node_visitor {
private:
  std::function<void(node*)> node_callback = NULL;
  std::function<void(start_node*)> start_node_callback = NULL;

public:
  virtual ~node_visitor() {
  }

  virtual void visit(node *se) {
    if(node_callback != NULL) node_callback(se);
    _default();
  }

  virtual void visit(start_node *se) {
    if(start_node_callback != NULL) start_node_callback(se);
    _default();
  }

  virtual void _default() {
  }

  void _(std::function<void(node*)> node_callback) {
    this->node_callback = node_callback;
  }

  void _(std::function<void(start_node*)> start_node_callback) {
    this->start_node_callback = start_node_callback;
  }
};

}
