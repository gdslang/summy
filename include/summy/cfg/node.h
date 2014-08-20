/*
 * node.h
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#pragma once

#include <stdlib.h>
#include <iostream>

namespace cfg {

class node {
private:
  size_t id;
public:
  node(size_t id) :
      id(id) {
  }
  virtual ~node() {
  }

  virtual void dot(std::ostream &stream);
};

}
