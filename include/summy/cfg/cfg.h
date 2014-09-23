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
#include <functional>

#include <cppgdsl/rreil/statement/statement.h>

namespace cfg {

class node;
class edge;
class bfs_iterator;

class cfg {
  friend class bfs_iterator;
private:
  std::vector<node*> nodes;
  std::vector<std::map<size_t, edge*>*> edges;

  void add_node(node *n);
public:
  cfg(std::vector<std::tuple<uint64_t, std::vector<gdsl::rreil::statement*>*>> &translated_binary);
  ~cfg();

  size_t add_nodes(std::vector<gdsl::rreil::statement*>* statements, size_t from_node);

  size_t next_node_id();
  size_t node_count();
  node *get_node(size_t id);

  size_t create_node(std::function<class node*(size_t)> constr);

  /*
   * Caution: edge map may get changed
   */
  std::map<size_t, edge*> *out_edges(size_t id);

  bfs_iterator begin();
  bfs_iterator end();

  void dot(std::ostream &stream);
};

}
