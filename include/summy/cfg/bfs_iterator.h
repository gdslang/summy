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
private:
  cfg &_cfg;
  std::set<size_t> seen;
  std::queue<size_t> next;

  bfs_iterator(cfg &cfg);
public:
  node *operator*();
  bfs_iterator &operator++();
};

}
