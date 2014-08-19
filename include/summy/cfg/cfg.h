/*
 * cfg.h
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#pragma once

#include <iosfwd>
#include <vector>
#include <map>

namespace cfg {

class node;
class edge;

class cfg {
private:
  std::vector<node> nodes;
  std::vector<std::map<size_t, edge*>> edges;
public:
  cfg(std::vector<node> nodes, std::vector<std::map<size_t, edge*>> edges) : nodes(nodes), edges(edges) {
  }

  void dot(std::ostream &stream);
};


}
