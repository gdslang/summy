/*
 * edge.h
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#pragma once

#include <stdint.h>
#include <iostream>
#include <cppgdsl/rreil/statement/statement.h>

namespace cfg {

class edge {
private:
  uint64_t address;


public:
  void dot(std::ostream &stream);
};

}


