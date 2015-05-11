/*
 * cfg.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#include <summy/cfg/cfg.h>
#include <iostream>
#include <stdlib.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/node/node.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/edge/edge_copy_visitor.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node_copy_visitor.h>
#include <summy/cfg/observer.h>
#include <map>

using namespace std;

bool cfg::edge_id::operator <(const edge_id &other) const {
  if(from < other.from) return true;
  else if(from > other.from) return false;
  return to < other.to;
}

bool cfg::edge_id::operator ==(const edge_id &other) const {
  return from == other.from && to == other.to;
}

std::ostream &cfg::operator <<(std::ostream &out, const edge_id &_this) {
  out << "(" << _this.from << " -> " << _this.to << ")";
  return out;
}

cfg::update_pop::update_pop(class cfg &cfg) : cfg(cfg) {
}

cfg::update_pop::~update_pop() {
  cfg.pop_updates();
}

void cfg::cfg::add_node(node *n) {
  node_payloads.push_back(n);
  edge_payloads.push_back(new edge_payloads_t());
  _in_edges.resize(_in_edges.size() + 1);
}

cfg::cfg::cfg() {
  updates_stack.push(std::vector<update>());
}

cfg::cfg::~cfg() {
  for(auto node : node_payloads)
    delete node;
  for(auto node_edges : edge_payloads) {
    for(auto edge_it : *node_edges)
      delete edge_it.second;
    delete node_edges;
  }
}

void cfg::cfg::add_program(translated_program_t &translated_binary) {
  for(auto elem : translated_binary) {
    size_t address;
    vector<gdsl::rreil::statement*> *statements;
    tie(address, statements) = elem;
    size_t from_node = create_node([&](size_t id) {
      return new address_node(id, address, DECODED);
    });
    add_nodes(statements, from_node);
  }
}

size_t cfg::cfg::add_nodes(std::vector<gdsl::rreil::statement*>* statements, size_t from_node) {
  size_t to_node = from_node;
  for(auto stmt : *statements) {
    to_node = create_node([&](size_t id) {
      return new node(id);
    });

    update_edge(from_node, to_node, new stmt_edge(stmt));

    from_node = to_node;
  }
  return to_node;
}

void cfg::cfg::update_destroy_edge(size_t from, size_t to, const edge *edge) {
  auto it = edge_payloads[from]->find(to);
  if(it != edge_payloads[from]->end()) {
    updates_stack.top().push_back(update { update_kind::UPDATE, from, to });
    delete it->second;
    it->second = edge;
  } else {
    updates_stack.top().push_back(update { update_kind::INSERT, from, to });
    edge_payloads[from]->operator [](to) = edge;
  }
  _in_edges[to].insert(from);
}

void cfg::cfg::update_edge(size_t from, size_t to, const edge *edge) {
  auto it = edge_payloads[from]->find(to);
  if(it != edge_payloads[from]->end()) {
    updates_stack.top().push_back(update { update_kind::UPDATE, from, to });
    it->second = edge;
  } else {
    updates_stack.top().push_back(update { update_kind::INSERT, from, to });
    edge_payloads[from]->operator [](to) = edge;
  }
  _in_edges[to].insert(from);
}

void cfg::cfg::erase_edge(size_t from, size_t to) {
  edge_payloads[from]->erase(to);
  updates_stack.top().push_back(update { update_kind::ERASE, from, to });
  _in_edges[to].erase(from);
}

void cfg::cfg::erase_destroy_edge(size_t from, size_t to) {
  delete edge_payloads[from]->operator [](to);
  edge_payloads[from]->erase(to);
  updates_stack.top().push_back(update { update_kind::ERASE, from, to });
  _in_edges[to].erase(from);
}

void cfg::cfg::register_observer(observer *o) {
  observers.push_back(o);
}

void cfg::cfg::unregister_observer(observer *o) {
  std::vector<observer*> obs_new;
  for(auto mine : observers)
    if(mine != o)
      obs_new.push_back(mine);
  observers = obs_new;
}

//void cfg::cfg::clear_updates() {
//  updates.clear();
//}

void cfg::cfg::commit_updates() {
  while(true) {
    if(!updates_stack.size())
      break;
    vector<update> updates = updates_stack.top();
    updates_stack.top().clear();
    if(updates.size() == 0) break;
    for(auto &o : observers)
      o->notify(updates);
  }
}

void cfg::cfg::clear_updates() {
  updates_stack.top().clear();
}

void cfg::cfg::pop_updates() {
  auto popped = updates_stack.top();
  updates_stack.pop();
  vector<update> &top_new = updates_stack.top();
  top_new.insert(top_new.end(), popped.begin(), popped.end());
}

cfg::update_pop cfg::cfg::push_updates() {
  updates_stack.push(std::vector<update>());
  return update_pop { *this };
}

void cfg::cfg::dot(std::ostream &stream, node_callback_t node_cb, edge_callback_t edge_cb) {
  stream << "digraph G {" << endl;
  for(auto node : node_payloads) {
    stream << "  ";
    node_cb(*node, stream);
    stream << endl;
  }
  stream << endl;
  for(size_t i = 0; i < edge_payloads.size(); i++) {
    auto &c = *edge_payloads[i];
    for(auto it = c.begin(); it != c.end(); it++) {
      stream << "  " << node_payloads[i]->get_id() << " -> " << node_payloads[it->first]->get_id() << " [label=";
      edge_cb(edge_id(i, it->first), stream);
      stream << "];" << endl;
    }
  }
  stream << "}" << endl;
}

void cfg::cfg::dot(std::ostream &stream, node_callback_t node_cb) {
  dot(stream, node_cb, [&](edge_id eid, std::ostream &stream) {
    edge_payloads[eid.from]->at(eid.to)->dot(stream);
  });
}

void cfg::cfg::dot(std::ostream &stream) {
  dot(stream, &node::dot);
}

size_t cfg::cfg::next_node_id() {
  return node_payloads.size();
}

size_t cfg::cfg::node_count() {
  return node_payloads.size();
}

bool cfg::cfg::contains(size_t node) {
  return node < node_payloads.size();
}

bool cfg::cfg::contains_edge(size_t from, size_t to) {
  return from < node_payloads.size() && edge_payloads[from]->find(to) != edge_payloads[from]->end();
}

cfg::node *cfg::cfg::get_node_payload(size_t id) {
  return node_payloads[id];
}

void cfg::cfg::replace_node_payload(node *n) {
  if(n->get_id() >= node_payloads.size())
    throw new string("cfg::cfg::replace_node_payload(size_t, node*): Invalid node payload replacement");
  delete node_payloads[n->get_id()];
  for(auto in : in_edges(n->get_id()))
    updates_stack.top().push_back(update { update_kind::UPDATE, in, n->get_id() });
  node_payloads[n->get_id()] = n;
}

size_t cfg::cfg::create_node(std::function<class node*(size_t)> constr) {
  size_t id = next_node_id();
  add_node(constr(id));
  return id;
}

cfg::edge_payloads_t const* cfg::cfg::out_edge_payloads(size_t id) {
  return edge_payloads[id];
}

const cfg::in_edges_t &cfg::cfg::in_edges(size_t id) {
  return _in_edges[id];
}

void cfg::cfg::merge(class cfg &other, size_t merge_node, size_t other_merge_node) {
  if(merge_node > node_payloads.size() || other_merge_node > other.node_payloads.size())
    throw string("Invalid merge node(s)");

  size_t offset = node_count();

  auto mapped_id = [&](size_t id) {
    if(id < other_merge_node)
      return offset + id;
    else if(id == other_merge_node)
      return merge_node;
    else
      return offset + id - 1;
  };

  delete node_payloads[merge_node];

  /*
   * Copy nodes
   */
  node_payloads.resize(offset + other.node_payloads.size() - 1);
  for(size_t i = 0; i < other.node_payloads.size(); i++) {
    node_copy_visitor ncv;
    ncv._node_id([&](size_t current_id) {
      return mapped_id(current_id);
    });
    other.node_payloads[i]->accept(ncv);
    node *n = ncv.get_node();
    node_payloads[n->get_id()] = n;
  }

  /*
   * Copy edges
   */
  edge_payloads.resize(offset + other.edge_payloads.size() - 1);
  for(size_t i = offset; i < edge_payloads.size(); i++)
    edge_payloads[i] = new edge_payloads_t();
  for(size_t i = 0; i < other.edge_payloads.size(); i++) {
    auto &dst_edges = *edge_payloads[mapped_id(i)];
    for(auto &edge_mapping : *other.edge_payloads.at(i)) {
      edge_copy_visitor ecv;
      edge_mapping.second->accept(ecv);
      edge *e = ecv.get_edge();
      dst_edges[mapped_id(edge_mapping.first)] = e;
      updates_stack.top().push_back(update { update_kind::INSERT, mapped_id(i), mapped_id(edge_mapping.first) });
    }
  }

  /*
   * Copy in_edges
   */
  _in_edges.resize(offset + other._in_edges.size() - 1);
  for(size_t i = 0; i < other._in_edges.size(); i++)
    for(auto in_edge : other._in_edges[i])
      _in_edges[mapped_id(i)].insert(mapped_id(in_edge));
}

cfg::bfs_iterator cfg::cfg::begin(size_t from, bool backwards) {
  return bfs_iterator(this, from, backwards);
}

cfg::bfs_iterator cfg::cfg::begin() {
  return bfs_iterator(this);
}

cfg::bfs_iterator cfg::cfg::end() {
  return bfs_iterator(this, true);
}

cfg::edge_set_t cfg::cfg::adjacencies(std::set<size_t> nodes) {
  std::set<std::tuple<size_t, size_t>> result;
  for(auto node : nodes) {
    auto const &out_edges = this->out_edge_payloads(node);
    for(auto &mapping : *out_edges)
      result.insert(tuple<size_t, size_t>(node, mapping.first));
    for(auto &incoming : _in_edges[node])
      result.insert(tuple<size_t, size_t>(incoming, node));
  }
  return result;
}

cfg::bfs_iterator cfg::cfg_view::begin() {
  if(rooted)
    return cfg->begin(root, backwards);
  else
    return cfg->begin();
}

cfg::bfs_iterator cfg::cfg_view::end() {
  return cfg->end();
}
