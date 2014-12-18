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

  expr_acc exp_glob(kind::AND, man);

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

      auto build = [&](auto stmt) {
        Expr e = smtb.build(stmt);
        Expr e_def = smt_defb.build(stmt);
        Expr comb = man.mkExpr(kind::AND, e, e_def);
        exp_edge.add(comb);
      };

      auto handle_assignment = [&](auto a) {
        bool live = false;
        sr::visitor srv;
        srv._default([&](id *_id) {
          shared_ptr<id> lhs_id_wrapped(_id, [&](void *x) {});
          if(lv_result.contains(to_id, lhs_id_wrapped, 0, 64))
            live = true;
        });
        a->get_lhs()->accept(srv);
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
        build(pe->get_memory());
      });
      _edge->accept(ev);

      if(!exp_edge.empty)
        exp_node.add(exp_edge.acc);

      state[*from][to_id] = exp_edge.acc;
    }

    if(!exp_node.empty)
      exp_glob.add(exp_node.acc);
  }

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

//    auto test_var = [&](string var) {
//      Expr test = context.var(var);
//      Expr unpack_test = man.mkExpr(kind::BITVECTOR_TO_NAT, test);
//      cout << "\e[1m\e[31m" << test << " := " << se.getValue(unpack_test) << "\e[0m" << endl;
//    };
//    auto test_mem = [&](string ptr, size_t rev) {
//      Expr pointer = context.var(ptr);
//      Expr mem_dref = man.mkExpr(kind::SELECT, context.memory(rev), man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(63, 3)), pointer));
//      Expr mem_test = man.mkExpr(kind::BITVECTOR_TO_NAT, mem_dref);
//      cout << "\e[1m\e[31m" << mem_dref << " := " << se.getValue(mem_test) << "\e[0m" << endl;
//    };
//    auto test_mem_def = [&](string ptr, size_t rev) {
//      Expr pointer = context.var(ptr);
//      Expr mem_dref = man.mkExpr(kind::SELECT, context.memory_def(rev), man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(63, 3)), pointer));
//      Expr mem_test = man.mkExpr(kind::BITVECTOR_TO_NAT, mem_dref);
//      cout << "\e[1m\e[31m" << mem_dref << " := " << se.getValue(mem_test) << "\e[0m" << endl;
//    };
//    auto test_mem_both = [&](string ptr, size_t rev) {
//      test_mem(ptr, rev);
//      test_mem_def(ptr, rev);
//    };

//    test_var("t0_142");
//    test_var("t0_176");
//    test_var("A_143");
//    test_var("B_201");
//    test_mem_both("BP", 49);
//    test_mem_both("BP", 51);
//    test_mem_both("BP", 203);
//    test_mem_both("BP", 201);
//    test_mem_both("BP", 103);

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

//    cout << endl << "---------------" << endl << endl;
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
