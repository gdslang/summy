/*
 * cfg.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#include <summy/cfg/cfg.h>
#include <iostream>
#include <stdlib.h>
#include <summy/cfg/address_node.h>

#include <summy/cfg/edge.h>
#include <summy/cfg/node.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/observer.h>
#include <map>

using namespace std;

cfg::update_pop::update_pop(class cfg &cfg) : cfg(cfg) {
}

cfg::update_pop::~update_pop() {
  cfg.pop_updates();
}

void cfg::cfg::add_node(node *n) {
  nodes.push_back(n);
  edges.push_back(new edges_t());
}

cfg::cfg::cfg(std::vector<std::tuple<uint64_t, std::vector<gdsl::rreil::statement*>*>> &translated_binary) {
  updates_stack.push(std::vector<update>());

  for(auto elem : translated_binary) {
    size_t address;
    vector<gdsl::rreil::statement*> *statements;
    tie(address, statements) = elem;
    size_t from_node = create_node([&](size_t id) {
      return new address_node(id, address);
    });
    add_nodes(statements, from_node);
  }
}

cfg::cfg::~cfg() {
  for(auto node : nodes)
    delete node;
  for(auto node_edges : edges) {
    for(auto edge_it : *node_edges)
      delete edge_it.second;
    delete node_edges;
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
  auto it = edges[from]->find(to);
  if(it != edges[from]->end())
    delete it->second;
  it->second = edge;
  updates_stack.top().push_back(update { update_kind::INSERT, from, to });
}

void cfg::cfg::update_edge(size_t from, size_t to, const edge *edge) {
  edges[from]->operator [](to) = edge;
  updates_stack.top().push_back(update { update_kind::INSERT, from, to });
}

void cfg::cfg::erase_edge(size_t from, size_t to) {
  edges[from]->erase(to);
  updates_stack.top().push_back(update { update_kind::ERASE, from, to });
}

void cfg::cfg::erase_destroy_edge(size_t from, size_t to) {
  delete edges[from]->operator [](to);
  edges[from]->erase(to);
  updates_stack.top().push_back(update { update_kind::ERASE, from, to });
}

void cfg::cfg::register_observer(observer *o) {
  observers.push_back(o);
}

void cfg::cfg::unregister_observer(observer *o) {
  throw string("Unimplemented");
//  auto o_it = observers.find(o);
//  if(o_it != observers.end())
//    observers.erase(o_it);
}

//void cfg::cfg::clear_updates() {
//  updates.clear();
//}

void cfg::cfg::commit_updates() {
  while(true) {
    vector<update> updates = updates_stack.top();
    updates_stack.top().clear();
    if(updates.size() == 0) break;
    for(auto &o : observers)
      o->notify(updates);
    updates.clear();
  }
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

void cfg::cfg::dot(std::ostream &stream) {
  stream << "digraph G {" << endl;
  for(auto node : nodes) {
    stream << "  ";
    node->dot(stream);
    stream << endl;
  }
  stream << endl;
  for(size_t i = 0; i < edges.size(); i++) {
    auto &c = *edges[i];
    for(auto it = c.begin(); it != c.end(); it++) {
      stream << "  " << nodes[i]->get_id() << " -> " << nodes[it->first]->get_id() << " [label=";
      it->second->dot(stream);
      stream << "];" << endl;
    }
  }
  stream << "}" << endl;
}

size_t cfg::cfg::next_node_id() {
  return nodes.size();
}

size_t cfg::cfg::node_count() {
  return nodes.size();
}

bool cfg::cfg::contains(size_t node) {
  return node < nodes.size();
}

bool cfg::cfg::contains_edge(size_t from, size_t to) {
  return from < nodes.size() && edges[from]->find(to) != edges[from]->end();
}

cfg::node *cfg::cfg::get_node(size_t id) {
  return nodes[id];
}

size_t cfg::cfg::create_node(std::function<class node*(size_t)> constr) {
  size_t id = next_node_id();
  add_node(constr(id));
  return id;
}

cfg::edges_t const* cfg::cfg::out_edges(size_t id) {
  return edges[id];
}

cfg::bfs_iterator cfg::cfg::begin() {
  return bfs_iterator(this);
}

cfg::bfs_iterator cfg::cfg::end() {
  return bfs_iterator(this, true);
}




