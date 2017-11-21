/*
 * node_copy_visitor.h
 *
 *  Created on: Oct 29, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/node/node_visitor.h>
#include <functional>
#include <optional>
#include "address_node.h"

namespace cfg {

class node;

class node_copy_visitor : public node_visitor {
public:
  typedef std::function<node*(size_t)> node_ctor_t;
  typedef std::function<address_node*(size_t, size_t, decoding_state, std::optional<std::string>)> address_node_ctor_t;
  typedef std::function<size_t(size_t)> node_id_ctor_t;
private:
  node_ctor_t node_ctor = NULL;
  address_node_ctor_t address_node_ctor = NULL;
  node_id_ctor_t node_id_ctor = NULL;

  size_t node_id(node *n);
protected:
  union {
    node *_node;
  };
public:
  node *get_node() {
    return _node;
  }

  virtual void visit(node *n);
  virtual void visit(address_node *n);

  void _(node_ctor_t c) {
    this->node_ctor = c;
  }
  void _(address_node_ctor_t c) {
    this->address_node_ctor = c;
  }
  void _node_id(node_id_ctor_t c) {
    this->node_id_ctor = c;
  }
};

}

