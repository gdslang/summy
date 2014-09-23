/*
 * trivial_connector.cpp
 *
 *  Created on: Sep 22, 2014
 *      Author: Julian Kranz
 */

#include <cppgdsl/rreil/copy_visitor.h>
#include <summy/transformers/trivial_connector.h>
#include <summy/cfg/node_visitor.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/edge.h>
#include <summy/cfg/node.h>
#include <summy/cfg/start_node.h>
#include <summy/tools/rreil_evaluator.h>
#include <cppgdsl/rreil/rreil.h>
#include <vector>
#include <queue>
#include <tuple>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;

trivial_connector::start_node_map_t trivial_connector::start_node_map() {
  trivial_connector::start_node_map_t start_node_map;
  for(auto node : *cfg) {
    node_visitor nv;
    nv._([&](start_node *sn) {
      start_node_map[sn->get_address()] = sn->get_id();
    });
    node->accept(nv);
  }
  return start_node_map;
}

void trivial_connector::transform() {
  auto start_node_map = this->start_node_map();
  queue<tuple<size_t, linear*>> branches;
  queue<tuple<size_t, sexpr*, bool, linear*>> cond_branches;

  for(auto node : *cfg) {
    auto &edges = *cfg->out_edges(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      bool replace = false;
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        v._([&](branch *i) {
          replace = true;
          copy_visitor cv;
          i->get_target()->get_lin()->accept(cv);
          branches.push(make_tuple(edge_it->first, cv.get_linear()));
        });
        v._([&](cbranch *i) {
          replace = true;
          auto branch = [&](bool then, address *branch) {
            copy_visitor cv;
            i->get_cond()->accept(cv);
            sexpr *cond = cv.get_sexpr();
            branch->get_lin()->accept(cv);
            cond_branches.push(make_tuple(edge_it->first, cond, then, cv.get_linear()));
          };
          branch(true, i->get_target_true());
          branch(false, i->get_target_false());
        });
        stmt->accept(v);
      });
      edge_it->second->accept(ev);
      if(replace) {
        delete edge_it->second;
        edges[edge_it->first] = new edge();
      }
    }
  }

  while(!branches.empty()) {
    size_t node_id;
    linear *lin;
    tie(node_id, lin) = branches.front();
    branches.pop();

    rreil_evaluator rev;
    bool evalable;
    int_t value;
    tie(evalable, value) = rev.evaluate(lin);

    if(evalable) {
      auto it = start_node_map.find(value);
      if(it != start_node_map.end()) {
        size_t dest_node_id = it->second;

        statement *ip_assign = new assign(64, new variable(new arch_id("IP"), 0),
            new expr_sexpr(new sexpr_lin(new lin_imm(value))));

        auto &edges = *cfg->out_edges(node_id);
        edges[dest_node_id] = new stmt_edge(ip_assign);

        delete ip_assign;
      }
    }

    delete lin;
  }

  while(!cond_branches.empty()) {
    size_t node_id;
    bool positive;
    sexpr *cond;
    linear *lin;
    tie(node_id, cond, positive, lin) = cond_branches.front();
    cond_branches.pop();

    rreil_evaluator rev;
    bool evalable;
    int_t value;
    tie(evalable, value) = rev.evaluate(lin);

    if(evalable) {
      auto it = start_node_map.find(value);
      if(it != start_node_map.end()) {
        size_t dest_node_id = it->second;

        class node *cond_end_node = new (class node)(cfg->next_node_id());
        cfg->add_node(cond_end_node);

        auto &edges_node = *cfg->out_edges(node_id);
        edges_node[cond_end_node->get_id()] = new cond_edge(cond, positive);

        statement *ip_assign = new assign(64, new variable(new arch_id("IP"), 0),
            new expr_sexpr(new sexpr_lin(new lin_imm(value))));

        auto &edges_cond_end = *cfg->out_edges(cond_end_node->get_id());
        edges_cond_end[dest_node_id] = new stmt_edge(ip_assign);

        delete ip_assign;
      }
    }

    delete cond;
    delete lin;
  }
}
