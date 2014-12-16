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
using namespace analysis;
using namespace cfg;
using namespace gdsl::rreil;
using namespace CVC4;
namespace sr = summy::rreil;

analysis::ismt::ismt(class cfg *cfg, liveness::liveness_result lv_result, adaptive_rd::adaptive_rd_result rd_result) :
    cfg(cfg), lv_result(lv_result), rd_result(rd_result), context(false), smtb(context, rd_result),
        smt_defb(context, rd_result) {
//  smtb = new smt_builder(context);
}

analysis::ismt::~ismt() {
//  delete smtb;
}

ismt_edge_ass_t analysis::ismt::analyse(size_t from) {
  struct target {
    Expr exp;
    Expr exp_def;
    edge_id edge;

    target(Expr exp, Expr exp_def, edge_id edge) : exp(exp), exp_def(exp_def), edge(edge) {
    }

    bool operator <(const target &other) const {
      return edge < other.edge;
    }
  };

//  auto target_comp = [](const target &a, const target &b) -> bool {
//    return a.edge < b.edge;
//  };
//  set<target, decltype(target_comp)> targets(target_comp);
  set<target> targets;

  ExprManager &man = context.get_manager();

  struct expr_acc {
    bool empty = true;
    Expr acc;
    kind::Kind_t kind;
    ExprManager &man;
    expr_acc(kind::Kind_t kind, ExprManager &man) :
        kind(kind), man(man) {
    }
    void add(Expr next) {
      if(empty) {
        empty = false;
        acc = next;
      } else {
        acc = man.mkExpr(kind, acc, next);
      }
    }
  };

  cout << "from: " << from << endl;

  expr_acc exp_glob(kind::AND, man);

  int i = 0;

  cfg_view cv(cfg, from, true);
  for(auto node : cv) {
    size_t to_id = node->get_id();
    expr_acc exp_node(kind::OR, man);


    auto &edges = cfg->in_edges(node->get_id());
    for(auto from = edges.begin(); from != edges.end(); from++) {
      if(!lv_result.edge_liveness[edge_id(*from, to_id)])
        continue;

      auto _edge = cfg->out_edge_payloads(*from)->at(to_id);
      smtb.edge(*from, to_id);
      smt_defb.edge(*from, to_id);
      expr_acc exp_edge(kind::AND, man);

//      if(i++ >= 32)
//        goto foo;
//      if(i >= 25)
//      cout << i << "~ " << *from << " ==> " << to_id << endl;

      auto build = [&](auto stmt) {
//        if(i < 25 || i == 27 || i == 28 || i == 29 || i == 30 || i == 31)
//          return;
        Expr e = smtb.build(stmt);
        Expr e_def = smt_defb.build(stmt);
        cout << "Kante: " <<  e << endl;
        cout << "Kante def: " <<  e_def << endl;
//          cout << *stmt << ": " << e_def << endl;
        Expr comb = man.mkExpr(kind::AND, e, e_def);
        exp_edge.add(comb);
      };

      auto handle_assignment = [&](auto a) {
        bool live = false;
        sr::visitor srv;
//          cout << *ass << ":: ";
        srv._default([&](id *_id) {
//            cout << *_id << ", ";
          shared_ptr<id> lhs_id_wrapped(_id, [&](void *x) {});
          if(lv_result.contains(to_id, lhs_id_wrapped, 0, 64))
            live = true;
        });
        a->get_lhs()->accept(srv);
//          cout << endl;
        if(live)
          build(a);
      };

      edge_visitor ev;
      ev._([&](const stmt_edge *sedge) {
        statement *stmt = sedge->get_stmt();
        statement_visitor sv;
        sv._([&](assign *ass) {
          handle_assignment(ass);
        });
        sv._([&](load *l) {
          handle_assignment(l);
        });
        sv._([&](store *s) {
          build(s);
        });
        sv._([&](branch *b) {
          targets.insert(target(smtb.build(b->get_target()), smt_defb.build_target(b->get_target()), edge_id(*from, to_id)));
        });
        stmt->accept(sv);
      });
      ev._([&](const phi_edge *pe) {
        for(auto &ass : pe->get_assignments())
          handle_assignment(&ass);
      });
      _edge->accept(ev);

      if(!exp_edge.empty)
        exp_node.add(exp_edge.acc);

      state[*from][to_id] = exp_edge.acc;
    }

//    if(from == 223 && to_id == 28)
//      break;
    if(!exp_node.empty)
      exp_glob.add(exp_node.acc);
  }
    foo:;

  ismt_edge_ass_t assignments;
  if(exp_glob.empty)
    return assignments;

  SmtEngine &se = context.get_smtEngine();

  cout << exp_glob.acc << endl;

//  se.assertFormula(man.mkExpr(kind::))

  /*
   * Todo: Enforce defined targets separately
   */
  for(auto &target : targets) {
    cout << "Asserting " << target.exp_def << endl;
    se.assertFormula(target.exp_def);
  }

  Result r;
  size_t max = 10;
  while(--max) {
    r = se.checkSat(exp_glob.acc);
//    cout << exp_glob.acc << " is " << r << endl;
    if(r.isSat() != Result::SAT) {
//      cout << "Unsat core: " << se.getUnsatCore() << endl;
      break;
    }

    for(auto &target : targets) {
      Expr var_exp = target.exp;
      Expr unpack = man.mkExpr(kind::BITVECTOR_TO_NAT, var_exp);
      cout << "\e[1m\e[31m" << var_exp << " := " << se.getValue(unpack) << "\e[0m" << endl;
//      cout << "\e[1m\e[31m" << context.var("A_26_def") << " := " << se.getValue(context.var("A_26_def")) << "\e[0m" << endl;

      assignments[target.edge].insert(stoull(se.getValue(unpack).toString()));
    }
    for(auto target : targets) {
      Expr v = target.exp;
      se.assertFormula(man.mkExpr(kind::DISTINCT, v, se.getValue(v)));
    }
  }

  if(!max)
    return ismt_edge_ass_t();

  return assignments;
}

void analysis::ismt::dot(std::ostream &stream) {
  stream << "digraph G {" << endl;
  for(auto node : *cfg) {
    stream << "  ";
    node->dot(stream);
    stream << endl;
    auto const &edges = cfg->out_edge_payloads(node->get_id());
    for(auto to = edges->begin(); to != edges->end(); to++) {
      stream << "  " << node->get_id() << " -> " << to->first << " [label=\"";
      stream << state[node->get_id()][to->first];
      stream << "\"];" << endl;
    }
  }
  stream << "}" << endl;
}
