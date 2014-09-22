/*
 * ip_propagator.cpp
 *
 *  Created on: Sep 21, 2014
 *      Author: jucs
 */

#include <summy/transformers/ip_propagator.h>
#include <summy/cfg/edge.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/node.h>
#include <summy/cfg/start_node.h>
#include <summy/cfg/node_visitor.h>
#include <summy/tools/rreil_evaluator.h>
#include <cppgdsl/rreil/copy_visitor.h>
#include <cppgdsl/rreil/rreil.h>
#include <vector>
#include <functional>
#include <tuple>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;

std::tuple<bool, int_t> ip_propagator::evaluate(int_t ip_value, gdsl::rreil::expr *e) {
  rreil_evaluator re([&](variable *v) -> tuple<bool, int_t> {
    return make_tuple(is_ip(v), ip_value);
  });
  return re.evaluate(e);
}

bool ip_propagator::is_ip(gdsl::rreil::variable *v) {
  bool is_ip = false;
  id_visitor iv;
  iv._([&] (arch_id *ai) {
    if(ai->get_name() == "IP")
      is_ip = true;
  });
  v->get_id()->accept(iv);
  return is_ip;
}

std::vector<int_t> *ip_propagator::analyze_ip() {
  vector<int_t> *result = new vector<int_t>(cfg->node_count());
  vector<bool> *calculated = new vector<bool>(cfg->node_count(), false);
  for(auto node : *cfg) {
    size_t id = node->get_id();
    node_visitor nv;
    nv._([&](start_node *sn) {
      (*calculated)[id] = true;
      (*result)[id] = sn->get_address();
    });
    node->accept(nv);
    if(!(*calculated)[id])
      throw string("Unknown IP value");
    int_t ip_current = (*result)[id];
    auto &edges = *cfg->out_edges(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        v._([&](assign *i) {
          if(is_ip(i->get_lhs())) {
            bool evalable;
            tie(evalable, ip_current) = evaluate(ip_current, i->get_rhs());
            if(!evalable)
              throw string("Can't evaluate IP value :-(");
          }
        });
        stmt->accept(v);
      });
      edge_it->second->accept(ev);

      //class node *child = cfg->get_node(edge_it->first);
      size_t child_id = edge_it->first;
      if((*calculated)[child_id]) {
        if((*result)[child_id] != ip_current)
          throw string("There should be no different IP values for one node :-(...");
      }
      else {
        (*result)[child_id] = ip_current;
        (*calculated)[child_id] = true;
      }
    }
  }
  delete calculated;
  return result;
}

void ip_propagator::transform() {
  auto ips = analyze_ip();

  for(auto node : *cfg) {
    auto &edges = *cfg->out_edges(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        copy_visitor cv;
        cv._([&](variable *v) -> linear* {
          if(is_ip(v)) {
            delete v;
            return new lin_imm((*ips)[edge_it->first]);
          } else
            return new lin_var(v);
        });
        edge->get_stmt()->accept(cv);
        statement *stmt_mod = cv.get_statement();
        delete edge_it->second;
        edges[edge_it->first] = new stmt_edge(stmt_mod);
        delete stmt_mod;
      });
      ev._([&](cond_edge *edge) {
        /*
         * Todo
         */
      });
      edge_it->second->accept(ev);
    }
  }

  delete ips;
}
