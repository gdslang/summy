/*
 * start_node.h
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#pragma once

#include "node.h"
#include <experimental/optional>
#include <string>

namespace cfg {

enum decoding_state { DECODED, DECODABLE, UNDEFINED };

class address_node : public node {
private:
  size_t address;
  decoding_state decs;
  std::experimental::optional<std::string> name;

  virtual void put(std::ostream &out);
public:
  address_node(size_t id, size_t address, decoding_state decs) : node(id), address(address), decs(decs) {}

  address_node(size_t id, size_t address, decoding_state decs, std::experimental::optional<std::string> name)
      : node(id), address(address), decs(decs), name(name) {}

  size_t get_address() {
    return address;
  }

  decoding_state get_decs() {
    return decs;
  }

  //  void set_decoded(bool decoded) {
  //    return this->decoded = decoded;
  //  }

  virtual void dot(std::ostream &stream);
  virtual void accept(node_visitor &v);
};
}
