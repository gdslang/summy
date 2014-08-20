/*
 * node_iterator.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: Julian Kranz
 */

#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>

cfg::bfs_iterator::bfs_iterator(cfg &cfg) : _cfg(cfg) {
  next.push(0);
}

cfg::node* cfg::bfs_iterator::operator *() {
  return _cfg.nodes[next.front()];
}


cfg::bfs_iterator& cfg::bfs_iterator::operator ++() {
  return *this;
}
