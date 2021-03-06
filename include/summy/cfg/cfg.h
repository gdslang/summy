/*
 * cfg.h
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <tuple>
#include <stdint.h>
#include <functional>
#include <optional>
#include <memory>

#include <cppgdsl/rreil/statement/statement.h>
#include <cppgdsl/iterator.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/edge/edge.h>

namespace cfg {

class node;
class edge;
class bfs_iterator;
class observer;
class cfg;

enum update_kind {
  INSERT, ERASE, UPDATE
};

/*
 * Todo: Replace tuples with this struct
 */
struct edge_id {
  size_t from;
  size_t to;

  edge_id(size_t from, size_t to) : from(from), to(to) {
  }
  bool operator <(const edge_id &other) const;
  bool operator ==(const edge_id &other) const;
};
std::ostream &operator<<(std::ostream &out, const edge_id &_this);

/*
 * Todo: Verschiedene Typen für Kantenpayloads zusammenführen
 */
template<typename PAY_T>
using edge_payload_map_t = std::map<edge_id, PAY_T>;
//template<typename PAY_T>
//using edge_payloads_gen_t = std::map<size_t, PAY_T>;
//template<typename PAY_T>
//using edges_payloads_gen_t = std::vector<size_t, PAY_T>;
//typedef edge_payloads_gen_t<edge const*> edge_payloads_t;


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

typedef std::map<size_t, edge const*> edge_payloads_t;
typedef std::set<size_t> in_edges_t;
typedef std::vector<std::tuple<uint64_t, gdsl::iterable<gdsl::rreil::statement>>> translated_program_t;

class cfg {
  friend class bfs_iterator;
  friend struct update_pop;
private:
  std::vector<node*> node_payloads;
  std::vector<edge_payloads_t*> edge_payloads;
  std::vector<in_edges_t> _in_edges;

  std::stack<std::vector<update>> updates_stack;
  std::vector<observer*> observers;

  void add_node(node *n);
  void pop_updates();
public:
  cfg();
  ~cfg();

  template<typename T>
  size_t add_program(std::vector<std::tuple<uint64_t, T>> const& translated_binary, std::optional<std::string> name) {
    std::optional<size_t> head_node;
    for(auto const& elem : translated_binary) {
      size_t address = std::get<0>(elem);
      T const& statements = std::get<1>(elem);
      size_t from_node = create_node([&](size_t id) { return new address_node(id, address, DECODED, name); });
      if(!head_node) head_node = from_node;
      add_nodes(statements, from_node);
    }
    return head_node.value();
  }

  template<typename T>
  size_t add_program(std::vector<std::tuple<uint64_t, T>> const& translated_binary) {
    return add_program(translated_binary, std::nullopt);
  }

  template<typename T>
  T const* foo(std::unique_ptr<T> const& x) {
    return x.get();
  }

  template<typename T>
  T const* foo(T const& x) {
    return &x;
  }

  template<typename T>
  size_t add_nodes(T const& statements, size_t from_node) {
    size_t to_node = from_node;
    for(auto const& stmt : statements) {
      to_node = create_node([&](size_t id) { return new node(id); });
      update_edge(from_node, to_node, new stmt_edge(foo(stmt)));
      from_node = to_node;
    }
    return to_node;
  }

//  size_t add_program(translated_program_t &translated_binary);
//  size_t add_program(translated_program_t &translated_binary, std::optional<std::string> name);
//  size_t add_program(std::vector<std::tuple<uint64_t, gdsl::rreil::statements_t>> &translated_binary);
//  size_t add_program(std::vector<std::tuple<uint64_t, gdsl::rreil::statements_t>> &, std::optional<std::string> name);
//  size_t add_nodes(gdsl::iterable<gdsl::rreil::statement> statements, size_t from_node);

  size_t next_node_id();
  size_t node_count();
  bool contains(size_t node);
  bool contains_edge(size_t from, size_t to);
  node *get_node_payload(size_t id);
  void replace_node_payload(node *n);

  size_t create_node(std::function<class node*(size_t)> constr);

  edge_payloads_t const* out_edge_payloads(size_t id);
  in_edges_t const& in_edges(size_t id);
  void update_edge(size_t from, size_t to, const edge *edge);
  void update_destroy_edge(size_t from, size_t to, const edge *edge);
  void erase_edge(size_t from, size_t to);
  void erase_destroy_edge(size_t from, size_t to);

  void merge(class cfg &other, size_t merge_node, size_t other_merge_node);

  bfs_iterator begin(size_t from, bool backwards);
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
  std::unique_ptr<class cfg::cfg> machine_cfg(bool call_targets);

  typedef std::function<void(node&, std::ostream&)> node_callback_t;
  typedef std::function<void(edge_id, std::ostream&)> edge_callback_t;
  void dot(std::ostream &stream, node_callback_t node_cb, edge_callback_t edge_cb);
  void dot(std::ostream &stream, node_callback_t node_cb);
  void dot(std::ostream &stream);
};

class cfg_view {
private:
  class cfg *cfg;
  bool rooted;
  size_t root;
  bool backwards;
public:
  cfg_view(class cfg *cfg, size_t root, bool backwards) :
      cfg(cfg), rooted(true), root(root), backwards(backwards) {
  }
  cfg_view(class cfg *cfg, size_t root) :
      cfg(cfg), rooted(true), root(root), backwards(false) {
  }
  cfg_view(class cfg *cfg) :
      cfg(cfg), rooted(false), root(0), backwards(false) {
  }
  bfs_iterator begin();
  bfs_iterator end();
};

}
