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
  std::set<size_t> seen;
  std::queue<size_t> q;
  bool end;

  bfs_iterator(cfg *cfg);
  bfs_iterator(cfg *cfg, size_t init_id);
public:
  node *operator*();
  bfs_iterator &operator++();
  friend bool operator==(const bfs_iterator &a, const bfs_iterator &b);
  friend bool operator!=(const bfs_iterator &a, const bfs_iterator &b);
};

bool operator==(const bfs_iterator &a, const bfs_iterator &b);
bool operator!=(const bfs_iterator &a, const bfs_iterator &b);

}
