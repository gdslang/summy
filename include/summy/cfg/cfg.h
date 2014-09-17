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
class bfs_iterator;

class cfg {
  friend class bfs_iterator;
private:
  std::vector<node*> nodes;
  std::vector<std::map<size_t, edge*>> edges;

public:
  cfg(std::vector<std::tuple<uint64_t, std::vector<gdsl::rreil::statement*>*>> &translated_binary);
  ~cfg();

  void add_node(node *n);
  size_t add_nodes(std::vector<gdsl::rreil::statement*>* statements, size_t from_node);

  size_t next_node_id();
  node *get_node(size_t id);

  /*
   * Caution: edge map may get changed
   */
  std::map<size_t, edge*> &out_edges(size_t id);

  bfs_iterator begin();
  bfs_iterator end();

  void dot(std::ostream &stream);
};

}
