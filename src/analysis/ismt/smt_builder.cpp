/*
 * smt_builder.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/ismt/smt_builder.h>
#include <cppgdsl/rreil/rreil.h>
#include <string>

using namespace std;
using namespace CVC4;
using namespace analysis;
using namespace gdsl::rreil;

CVC4::Expr analysis::smt_builder::pop() {
  if(sub_exprs.empty())
    throw string(":/");
  Expr r = sub_exprs.back();
  sub_exprs.pop_back();
  return r;
}

void smt_builder::_default(gdsl::rreil::id *i) {
  auto i_str = i->to_string();
  auto &var_map = context.get_var_map();
  auto i_it = var_map.find(i_str);
  if(i_it != var_map.end())
    sub_exprs.push_back(i_it->second);
  else {
    auto &man = context.get_manager();
    Expr i_exp = man.mkVar(i_str, man.mkBitVectorType(64));
    var_map[i_str] = i_exp;
    sub_exprs.push_back(i_exp);
  }
}

//void analysis::smt_builder::visit(gdsl::rreil::variable *v) {
//}

void analysis::smt_builder::visit(gdsl::rreil::lin_binop *a) {
  auto &man = context.get_manager();
  a->get_opnd1()->accept(*this);
  Expr opnd1 = pop();
  a->get_opnd2()->accept(*this);
  Expr opnd2 = pop();
  Expr r;
  switch(a->get_op()) {
    case BIN_LIN_ADD: {
      r = man.mkExpr(kind::BITVECTOR_PLUS, opnd1, opnd2);
      break;
    }
    case BIN_LIN_SUB: {
      r = man.mkExpr(kind::BITVECTOR_SUB, opnd1, opnd2);
      break;
    }
  }
  sub_exprs.push_back(r);
}

void analysis::smt_builder::visit(gdsl::rreil::lin_imm *a) {
  auto &man = context.get_manager();
  Expr imm = man.mkConst(BitVector(64, (unsigned long int)a->get_imm()));
  sub_exprs.push_back(imm);
}

void analysis::smt_builder::visit(gdsl::rreil::lin_scale *a) {
  auto &man = context.get_manager();
  Expr factor_bv = man.mkConst(BitVector(64, (unsigned long int)a->get_const()));
  a->get_opnd()->accept(*this);
  Expr opnd = pop();
  Expr r = man.mkExpr(kind::BITVECTOR_MULT, factor_bv, opnd);
  sub_exprs.push_back(r);
}

void smt_builder::visit(gdsl::rreil::assign *a) {
//  base::visit(a);
  auto &man = context.get_manager();
  a->get_rhs()->accept(*this);
  Expr rhs = pop();
  a->get_lhs()->accept(*this);
  Expr lhs = pop();
  Expr ass = man.mkExpr(kind::EQUAL, rhs, lhs);
  sub_exprs.push_back(ass);
}

CVC4::Expr analysis::smt_builder::build(gdsl::rreil::statement *s) {
  s->accept(*this);
  return pop();
}
