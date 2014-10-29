/*
 * node_iterator.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: Julian Kranz
 */

#include <summy/cfg/address_node.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/node/node_visitor.h>
#include <string>

using std::string;

void cfg::bfs_iterator::check_next_component() {
  if(inner_component.empty()) while(!components.empty()) {
    size_t next_comp = components.front();
    components.pop();
    if(seen.find(next_comp) == seen.end()) {
      inner_component.push(next_comp);
      break;
    }
  }
}

cfg::bfs_iterator::bfs_iterator(cfg *cfg, bool end) :
    _cfg(cfg), end(end) {
}

cfg::bfs_iterator::bfs_iterator(cfg *cfg) :
    _cfg(cfg), end(false) {
  for(auto node : cfg->nodes) {
    node_visitor nv;
    nv._([&](address_node *sn) {
      components.push(sn->get_id());
    });
    node->accept(nv);
  }
  check_next_component();
}

cfg::node *cfg::bfs_iterator::operator *() {
  if(end) throw string("No more nodes");
  return _cfg->nodes[inner_component.front()];
}

cfg::bfs_iterator &cfg::bfs_iterator::operator ++() {
  if(end) throw string("No more nodes");
  size_t next = inner_component.front();
  inner_component.pop();
  auto &edges = *_cfg->out_edges(next);
  for(auto edge : edges)
    if(seen.find(edge.first) == seen.end()) {
      seen.insert(edge.first);
      inner_component.push(edge.first);
    }
  check_next_component();
  end = inner_component.empty() && components.empty();
  return *this;
}

bool cfg::operator ==(const bfs_iterator &a, const bfs_iterator &b) {
  return (a.end && b.end)
      || ((!a.end && !b.end)
          && (a.seen == b.seen && a.inner_component == b.inner_component && a._cfg == b._cfg
              && a.components == b.components));
}

bool cfg::operator !=(const bfs_iterator &a, const bfs_iterator &b) {
  return !(a == b);
}


