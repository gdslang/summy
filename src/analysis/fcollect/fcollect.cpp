/*
 * fcollect.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <cppgdsl/rreil/copy_visitor.h>
#include <cppgdsl/rreil/rreil.h>
#include <functional>
#include <memory>
#include <set>
#include <summy/analysis/domain_state.h>
#include <summy/analysis/fcollect/fcollect.h>
#include <summy/analysis/fcollect/fcollect_state.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/tools/rreil_util.h>
#include <tuple>
#include <vector>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;
using namespace analysis::fcollect;

std::map<size_t, std::shared_ptr<analysis::domain_state>> analysis::fcollect::fcollect::transform(
  size_t from, size_t to, const ::cfg::edge *e, size_t from_ctx) {
  constraint_t transfer_f = [=](size_t) { return default_context(get(0)); };
  edge_visitor ev;
  ev._([&](const stmt_edge *edge) {
    statement *stmt = edge->get_stmt();
    statement_visitor v;
    v._([&](branch const *b) {
      if(b->get_hint() == gdsl::rreil::branch_hint::BRANCH_HINT_CALL) {
        rreil_evaluator rev;
        bool success;
        size_t address;
        tie(success, address) = rev.evaluate(&b->get_target().get_lin());
        if(success) {
          state.insert(address);
        }
      }
    });
    stmt->accept(v);
  });
  e->accept(ev);
  known_nodes.insert(to);
  return transfer_f(from_ctx);
}

analysis::dependency analysis::fcollect::fcollect::gen_dependency(size_t from, size_t to) {
  return dependency{from, to};
}

void analysis::fcollect::fcollect::init_state() {}

fcollect::fcollect::fcollect(class cfg *cfg)
    : fp_analysis::fp_analysis(cfg, analysis_direction::FORWARD) {
  init();
}

analysis::fcollect::fcollect::~fcollect() {}

shared_ptr<::analysis::domain_state> fcollect::fcollect::get(size_t node) {
  if(known_nodes.find(node) != known_nodes.end())
    return make_shared<fcollect_state>(1);
  else
    return make_shared<fcollect_state>(0);
}

std::shared_ptr<::analysis::domain_state> fcollect::fcollect::start_state(size_t) {
  return make_shared<fcollect_state>(1);
}

void fcollect::update(analysis_node, shared_ptr<::analysis::domain_state>) {}

fcollect_result analysis::fcollect::fcollect::result() {
  return fcollect_result(state);
}

void analysis::fcollect::fcollect::put(std::ostream &out) {
  cout << "analysis::fcollect::fcollect::put" << endl;
}
