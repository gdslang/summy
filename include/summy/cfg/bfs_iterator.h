/*
 * node_iterator.h
 *
 *  Created on: Aug 20, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <iterator>
#include <vector>
#include <queue>
#include <set>
#include <summy/cfg/node.h>

namespace cfg {

class cfg;

class bfs_iterator: std::iterator<std::input_iterator_tag, node*> {
  friend class cfg;
private:
  cfg *_cfg;
  /**
   * Set of already visited nodes
   */
  std::set<size_t> seen;
  /**
   * Queue of nodes within current component
   */
  std::queue<size_t> inner_component;
  /**
   * Queue of start nodes that are possible component starting points
   */
  std::queue<size_t> components;
  bool end;

  /**
   * Move to next component if necessary
   */
  void check_next_component();

  bfs_iterator(cfg *cfg, bool end);
  bfs_iterator(cfg *cfg);
public:
  node *operator*();
  bfs_iterator &operator++();
  friend bool operator==(const bfs_iterator &a, const bfs_iterator &b);
  friend bool operator!=(const bfs_iterator &a, const bfs_iterator &b);
};

bool operator==(const bfs_iterator &a, const bfs_iterator &b);
bool operator!=(const bfs_iterator &a, const bfs_iterator &b);

}
