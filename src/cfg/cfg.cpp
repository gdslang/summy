/*
 * cfg.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#include <summy/cfg/cfg.h>
#include <iostream>
#include <stdlib.h>

#include <summy/cfg/edge.h>
#include <summy/cfg/node.h>
#include <summy/cfg/start_node.h>
#include <summy/cfg/bfs_iterator.h>
#include <map>

using namespace std;

void cfg::cfg::add_node(node *n) {
  nodes.push_back(n);
  edges.push_back(new map<size_t, edge*>());
}

cfg::cfg::cfg(std::vector<std::tuple<uint64_t, std::vector<gdsl::rreil::statement*>*>> &translated_binary) {
  for(auto elem : translated_binary) {
    size_t address;
    vector<gdsl::rreil::statement*> *statements;
    tie(address, statements) = elem;
    size_t from_node = create_node([&](size_t id) {
      return new start_node(id, address);
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

    map<size_t, edge*> &from_edges = *edges[from_node];
    from_edges[to_node] = new stmt_edge(stmt);

    from_node = to_node;
  }
  return to_node;
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

std::map<size_t, cfg::edge*> *cfg::cfg::out_edges(size_t id) {
  return edges[id];
}

cfg::bfs_iterator cfg::cfg::begin() {
  return bfs_iterator(this);
}

cfg::bfs_iterator cfg::cfg::end() {
  return bfs_iterator(this, true);
}
