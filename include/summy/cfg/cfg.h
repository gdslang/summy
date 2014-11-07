/*
 * cfg.h
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#pragma once

#include <iosfwd>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <tuple>
#include <stdint.h>
#include <functional>

#include <cppgdsl/rreil/statement/statement.h>

namespace cfg {

class node;
class edge;
class bfs_iterator;
class observer;
class cfg;

enum update_kind {
  INSERT, ERASE, UPDATE
};

struct update {
  update_kind kind;
  size_t from;
  size_t to;
};

typedef std::vector<update> updates_t;
typedef std::set<std::tuple<size_t, size_t>> edge_set_t;

struct update_pop {
private:
  class cfg &cfg;
public:
  update_pop(class cfg &cfg);
  ~update_pop();
};

typedef std::map<size_t, edge const*> edges_t;
typedef std::set<size_t> in_edges_t;
typedef std::vector<std::tuple<uint64_t, std::vector<gdsl::rreil::statement*>*>> translated_program_t;

class cfg {
  friend class bfs_iterator;
  friend struct update_pop;
private:
  std::vector<node*> nodes;
  std::vector<edges_t*> edges;
  std::vector<in_edges_t> _in_edges;

  std::stack<std::vector<update>> updates_stack;
  std::vector<observer*> observers;

  void add_node(node *n);
  void pop_updates();
public:
  cfg();
  ~cfg();

  void add_program(translated_program_t &translated_binary);
  size_t add_nodes(std::vector<gdsl::rreil::statement*> *statements, size_t from_node);

  size_t next_node_id();
  size_t node_count();
  bool contains(size_t node);
  bool contains_edge(size_t from, size_t to);
  node *get_node(size_t id);

  size_t create_node(std::function<class node*(size_t)> constr);

  edges_t const* out_edges(size_t id);
  in_edges_t const& in_edges(size_t id);
  void update_edge(size_t from, size_t to, const edge *edge);
  void update_destroy_edge(size_t from, size_t to, const edge *edge);
  void erase_edge(size_t from, size_t to);
  void erase_destroy_edge(size_t from, size_t to);

  void merge(class cfg &other, size_t merge_node, size_t other_merge_node);

  bfs_iterator begin(size_t from);
  bfs_iterator begin();
  bfs_iterator end();

  void register_observer(observer *o);
  void unregister_observer(observer *o);
  std::vector<update> const &get_updates() {
    return updates_stack.top();
  }
  void commit_updates();
  void clear_updates();
  update_pop push_updates();

  edge_set_t adjacencies(std::set<size_t> nodes);

  void dot(std::ostream &stream);
};

class cfg_view {
private:
  class cfg *cfg;
  bool rooted;
  size_t root;
public:
  cfg_view(class cfg *cfg, size_t root) :
      cfg(cfg), rooted(true), root(root) {
  }
  cfg_view(class cfg *cfg) :
      cfg(cfg), rooted(false), root(0) {
  }
  bfs_iterator begin();
  bfs_iterator end();
};

}
