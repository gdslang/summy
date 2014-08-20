/*
 * start_node.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: jucs
 */

#include <summy/cfg/start_node.h>

void cfg::start_node::dot(std::ostream &stream) {
  stream << address;
}
