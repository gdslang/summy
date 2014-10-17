/*
 * start_node.h
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#pragma once

#include "node.h"

namespace cfg {

class address_node: public node {
private:
  size_t address;
public:
  address_node(size_t id, size_t address) :
      node(id), address(address) {
  }

  size_t get_address() {
    return address;
  }

  virtual void dot(std::ostream &stream);
  virtual void accept(node_visitor &v);
};

}
