/*
 * start_node.h
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#pragma once

#include "node.h"

namespace cfg {

class start_node : public node {
private:
  size_t address;
public:
  start_node(size_t address) : address(address) {
  }
};

}
