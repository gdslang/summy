
/*
 * node.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: jucs
 */

#include <summy/cfg/node.h>

void cfg::node::dot(std::ostream &stream) {
  stream << id << ";";
}
