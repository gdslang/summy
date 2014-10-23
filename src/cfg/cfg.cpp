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
#include <map>

using namespace std;

void cfg::cfg::add_node(node *n) {
  nodes.push_back(n);
  edges.push_back(new edges_t());
}

cfg::cfg::cfg(std::vector<std::tuple<uint64_t, std::vector<gdsl::rreil::statement*>*>> &translated_binary) {
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

void cfg::cfg::clear_updates() {
  updates.clear();
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

void cfg::cfg::update_destroy_edge(size_t from, size_t to, const edge *edge) {
  auto it = edges[from]->find(to);
  if(it != edges[from]->end())
    delete it->second;
  it->second = edge;
}

void cfg::cfg::update_edge(size_t from, size_t to, const edge *edge) {
  edges[from]->operator [](to) = edge;
}

void cfg::cfg::erase_edge(size_t from, size_t to) {
  edges[from]->erase(to);
}

void cfg::cfg::erase_destroy_edge(size_t from, size_t to) {
  delete edges[from]->operator [](to);
  edges[from]->erase(to);
}

cfg::bfs_iterator cfg::cfg::begin() {
  return bfs_iterator(this);
}

cfg::bfs_iterator cfg::cfg::end() {
  return bfs_iterator(this, true);
}
