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

cfg::cfg::cfg(std::vector<std::tuple<uint64_t, std::vector<gdsl::rreil::statement*>*>> &translated_binary) {
  for(auto elem : translated_binary) {
    size_t address;
    vector<gdsl::rreil::statement*> *statements;
    tie(address, statements) = elem;
    nodes.push_back(new start_node(nodes.size(), address));
    for(auto stmt : *statements) {
      map<size_t, edge*> node_edges;
      size_t node_id = nodes.size();
      node_edges[node_id] = new edge(stmt);

      nodes.push_back(new node(nodes.size()));
      edges.push_back(node_edges);
    }
  }
  nodes.push_back(new node(nodes.size()));
}

cfg::cfg::~cfg() {
  for(auto node : nodes)
    delete node;
  for(auto node_edges : edges)
    for(auto edge_it : node_edges)
      delete edge_it.second;
}

void cfg::cfg::dot(std::ostream &stream) {
  stream << "digraph G {" << endl;
  for(size_t i = 0; i < edges.size(); i++) {
    auto &c = edges[i];
    for(auto it = c.begin(); it != c.end(); it++) {
      stream << "  ";
      nodes[i]->dot(stream);
      stream << " -> ";
      nodes[it->first]->dot(stream);
      stream << " [label=";
      it->second->dot(stream);
      stream << "];" << endl;
    }
  }
  stream << "}" << endl;
}

cfg::node *cfg::cfg::get_node(size_t id) {
  return nodes[id];
}

std::map<size_t, cfg::edge*> &cfg::cfg::out_edges(size_t id) {
  return edges[id];
}

cfg::bfs_iterator cfg::cfg::begin() {
  return bfs_iterator(*this);
}

bool cfg::cfg::end() {
  return true;
}
