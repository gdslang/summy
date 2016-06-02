/*
 * fcollect.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/fcollect/fcollect.h>
#include <summy/analysis/fcollect/fcollect_state.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/tools/rreil_util.h>
#include <cppgdsl/rreil/rreil.h>
#include <cppgdsl/rreil/copy_visitor.h>
#include <vector>
#include <set>
#include <functional>
#include <memory>
#include <assert.h>
#include <tuple>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;
using namespace analysis::fcollect;

void analysis::fcollect::fcollect::add_constraint(size_t from, size_t to, const ::cfg::edge *e) {
    function<shared_ptr<domain_state>()> transfer_f = [=]() {
      return get(0);
    };
    edge_visitor ev;
    ev._([&](const stmt_edge *edge) {
      statement *stmt = edge->get_stmt();
      statement_visitor v;
      v._([&](branch *b) {
        if(b->get_hint() == gdsl::rreil::branch_hint::BRANCH_HINT_CALL) {
          rreil_evaluator rev;
          bool success;
          size_t address;
          tie(success, address) = rev.evaluate(b->get_target()->get_lin());
          if(success) {
            state.insert(address);
          }
        }
      });
      stmt->accept(v);
    });
    e->accept(ev);
    (constraints[to])[from] = transfer_f;
}

void analysis::fcollect::fcollect::remove_constraint(size_t from, size_t to) {
  constraints[to].erase(from);
}

analysis::dependency analysis::fcollect::fcollect::gen_dependency(size_t from, size_t to) {
  return dependency{from, to};
}

void analysis::fcollect::fcollect::init_state() {}

fcollect::fcollect::fcollect(class cfg *cfg) : fp_analysis::fp_analysis(cfg) {
  auto begin = cfg->begin();
  auto end = cfg->end();
//  for(auto it = cfg->begin();;) {
//      break;
//
//  }
//  for(auto node : *cfg) {
//    size_t node_id = node->get_id();
//    auto &edges = *cfg->out_edge_payloads(node_id);
//    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
//      add_constraint(node_id, edge_it->first, edge_it->second);
//      auto dep = gen_dependency(node_id, edge_it->first);
//      assert_dependency(dep);
//    }
//  }
}

analysis::fcollect::fcollect::~fcollect() {}

shared_ptr<::analysis::domain_state> fcollect::fcollect::get(size_t node) {
  return make_shared<fcollect_state>();
}

void fcollect::update(size_t node, shared_ptr<::analysis::domain_state> state) {
}

fcollect_result analysis::fcollect::fcollect::result() {
  return fcollect_result(state);
}

void analysis::fcollect::fcollect::put(std::ostream &out) {
  cout << "analysis::fcollect::fcollect::put" << endl;
}
