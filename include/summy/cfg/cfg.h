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
#include <tuple>
#include <stdint.h>

#include <cppgdsl/rreil/statement/statement.h>

namespace cfg {

class node;
class edge;

class cfg {
private:
  std::vector<node*> nodes;
  std::vector<std::map<size_t, edge*>> edges;
public:
  cfg(std::vector<std::tuple<uint64_t, std::vector<gdsl::rreil::statement*>*>> &translated_binary);

  void dot(std::ostream &stream);
};


}
