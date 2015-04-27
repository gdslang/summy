/*
 * start_node.h
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#pragma once

#include "node.h"

namespace cfg {

enum decoding_state {
  DECODED, DECODABLE, UNDEFINED
};

class address_node: public node {
private:
  size_t address;
  bool decoded;
public:
  address_node(size_t id, size_t address, bool decoded) :
      node(id), address(address), decoded(decoded) {
  }

  size_t get_address() {
    return address;
  }

  bool get_decoded() {
    return decoded;
  }

//  void set_decoded(bool decoded) {
//    return this->decoded = decoded;
//  }

  virtual void dot(std::ostream &stream);
  virtual void accept(node_visitor &v);
};

}
