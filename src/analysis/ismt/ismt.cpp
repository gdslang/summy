/*
 * ismt.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#include <cvc4/cvc4.h>
#include <summy/analysis/ismt/ismt.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <cppgdsl/rreil/rreil.h>
#include <iostream>
#include <set>

using namespace std;
using namespace cfg;
using namespace gdsl::rreil;
using namespace CVC4;
namespace sr = summy::rreil;

analysis::ismt::ismt(class cfg *cfg, liveness::liveness_result lv_result, adaptive_rd::adaptive_rd_result rd_result) :
    cfg(cfg), lv_result(lv_result), rd_result(rd_result), smtb(context) {
//  smtb = new smt_builder(context);
}

analysis::ismt::~ismt() {
//  delete smtb;
}

void analysis::ismt::analyse(size_t from) {
  set<string> vars;

  ExprManager &man = context.get_manager();
  bool first = true;
  Expr acc;

  cfg_view cv(cfg, from, true);
  for(auto node : cv) {
    size_t node_id = node->get_id();
    auto &edges = cfg->in_edges(node->get_id());
    for(auto from = edges.begin(); from != edges.end(); from++) {
      auto _edge = cfg->out_edges(*from)->at(node_id);

      auto handle_assignment = [&](auto a, kind::Kind_t kind) {
        bool live = false;
        sr::visitor srv;
//          cout << *ass << ":: ";
        srv._default([&](id *_id) {
//            cout << *_id << ", ";
          shared_ptr<id> lhs_id_wrapped(_id, [&](void *x) {});
          if(lv_result.contains(node_id, lhs_id_wrapped, 0, 64))
            live = true;
        });
        a->get_lhs()->accept(srv);
//          cout << endl;
        if(live) {
          Expr e = smtb.build(a, rd_result.result[*from]);
//          cout << *a << ": " << e << endl;
          if(first) {
            first = false;
            acc = e;
          } else {
            acc = man.mkExpr(kind, acc, e);
          }
        }
      };

      edge_visitor ev;
      ev._([&](const stmt_edge *sedge) {
        statement *stmt = sedge->get_stmt();
        statement_visitor sv;
        sv._([&](assign *ass) {
          handle_assignment(ass, kind::AND);
        });
        sv._([&](branch *b) {
          vars.insert(b->get_target()->get_lin()->to_string());
        });
        stmt->accept(sv);
      });
      ev._([&](const phi_edge *pe) {
        for(auto &ass : pe->get_assignments())
          handle_assignment(&ass, kind::OR);
      });
      _edge->accept(ev);
    }
  }

  if(first)
    return;

  SmtEngine &se = context.get_smtEngine();

  Result r;
  size_t max = 10;
  while(--max) {
    r = se.checkSat(acc);
    cout << acc << " is " << r << endl;
    if(r.isSat() != Result::SAT)
      break;

    auto &var_map = context.get_var_map();
    for(auto var : vars) {
      Expr unpack = man.mkExpr(kind::BITVECTOR_TO_NAT, var_map[var]);
      cout << "\e[1m\e[31m" << var_map[var] << " := " << se.getValue(unpack) << "\e[0m" << endl;
    }
    for(auto var : vars) {
      Expr v = var_map[var];
      se.assertFormula(man.mkExpr(kind::DISTINCT, v, se.getValue(v)));
    }
  };
}
