/*
 * cfg.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#include <summy/cfg/cfg.h>
#include <iostream>
#include <stdlib.h>

#include <summy/cfg/edge.h>
#include <summy/cfg/node.h>

using namespace std;

void cfg::cfg::dot(std::ostream &stream) {
  stream << "digraph G {" << endl;
  for(size_t i = 0; i < edges.size(); i++) {
    auto &c = edges[i];
    for(auto it = c.begin(); it != c.end(); it++) {
      stream << "  " << i << " -> " << it->first << " [label=";
      it->second->dot(stream);
      stream << "];" << endl;
    }
  }
  stream << "}" << endl;
}
