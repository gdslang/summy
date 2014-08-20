/*
 * edge.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#include <summy/cfg/edge.h>

cfg::edge::~edge() {
}

void cfg::edge::dot(std::ostream &stream) {
  stream << "\"" << *stmt << "\"";
}
