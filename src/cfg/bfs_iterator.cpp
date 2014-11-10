/*
 * node_iterator.cpp
 *
 *  Created on: Aug 20, 2014
 *      Author: Julian Kranz
 */

#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node_visitor.h>
#include <string>
#include <assert.h>

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
  end = inner_component.empty() && components.empty();
}

cfg::bfs_iterator::bfs_iterator(cfg *cfg, bool end) :
    _cfg(cfg), end(end), backwards(false) {
  assert(end);
}

cfg::bfs_iterator::bfs_iterator(cfg *cfg) :
    _cfg(cfg), end(false), backwards(false) {
  for(auto node : cfg->nodes) {
    node_visitor nv;
    nv._([&](address_node *sn) {
      components.push(sn->get_id());
    });
    node->accept(nv);
  }
  check_next_component();
}

cfg::bfs_iterator::bfs_iterator(cfg *cfg, size_t from, bool backwards) :
    _cfg(cfg), end(false), backwards(backwards) {
  components.push(from);
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
  auto handle_edge = [&](size_t to) {
    if(seen.find(to) == seen.end()) {
      seen.insert(to);
      inner_component.push(to);
    }
  };
  if(backwards) {
    auto &edges = _cfg->in_edges(next);
    for(auto &edge : edges)
      handle_edge(edge);
  } else {
    auto &edges = *_cfg->out_edges(next);
    for(auto &edge : edges)
      handle_edge(edge.first);
  }
  check_next_component();
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


