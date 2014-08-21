/*
 * node_iterator.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: Julian Kranz
 */

#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>
#include <string>

using std::string;

cfg::bfs_iterator::bfs_iterator(cfg *cfg) :
    _cfg(cfg), end(true) {
}

cfg::bfs_iterator::bfs_iterator(cfg *cfg, size_t init_id) :
    _cfg(cfg), end(false) {
  q.push(init_id);
}

cfg::node *cfg::bfs_iterator::operator *() {
  return _cfg->nodes[q.front()];
}

cfg::bfs_iterator &cfg::bfs_iterator::operator ++() {
  if(q.empty()) throw string("No more nodes");
  size_t next = q.front();
  q.pop();
  auto edges = _cfg->out_edges(next);
  for(auto edge : edges)
    q.push(edge.first);
  end = q.empty();
  return *this;
}

bool cfg::operator ==(const bfs_iterator &a, const bfs_iterator &b) {
  return (a.end && b.end) || (a.seen == b.seen && a.q == b.q && a._cfg == b._cfg);
}

bool cfg::operator !=(const bfs_iterator &a, const bfs_iterator &b) {
  return !(a == b);
}
