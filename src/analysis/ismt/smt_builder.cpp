/*
 * smt_builder.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/ismt/smt_builder.h>
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/ssa_id.h>
#include <summy/tools/rreil_util.h>
#include <cppgdsl/rreil/rreil.h>
#include <string>
#include <vector>

using namespace std;
using namespace CVC4;
using namespace analysis;
using namespace gdsl::rreil;
namespace sr = summy::rreil;

CVC4::Expr analysis::smt_builder::pop() {
  if(sub_exprs.empty())
    throw string(":/");
  Expr r = sub_exprs.back();
  sub_exprs.pop_back();
  return r;
}

CVC4::Expr analysis::smt_builder::id_by_string(std::string s) {
  auto &var_map = context.get_var_map();
  auto i_it = var_map.find(s);
  if(i_it != var_map.end())
    return i_it->second;
  else {
    auto &man = context.get_manager();
    Expr i_exp = man.mkVar(s, man.mkBitVectorType(64));
    var_map[s] = i_exp;
    return i_exp;
  }
}

void smt_builder::_default(gdsl::rreil::id *i) {
  auto i_str = i->to_string();
  Expr i_exp = id_by_string(i_str);
  sub_exprs.push_back(i_exp);
}

void analysis::smt_builder::visit(gdsl::rreil::variable *v) {
  v->get_id()->accept(*this);
  if(v->get_offset() > 0 && rhs) {
    Expr c = pop();
    auto &man = context.get_manager();
    sub_exprs.push_back(man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(63, v->get_offset())), c));
  }
}

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
  rhs = true;
  a->get_rhs()->accept(*this);
  rhs = false;
  Expr rhs = pop();
  a->get_lhs()->accept(*this);
  Expr lhs = pop();

  int_t ass_size = rreil_prop::size_of_assign(a);
  int_t offset = a->get_lhs()->get_offset();

  Expr rhs_conc;
  if(ass_size == 0 || ass_size == 64) rhs_conc = rhs;
  else {
    shared_ptr<id> lhs_id_wrapped(a->get_lhs()->get_id(), [&](void *x) {});
    auto def_it = defs->get_elements().find(lhs_id_wrapped);
    string id_old_str;
    if(def_it != defs->get_elements().end()) id_old_str = def_it->first->to_string();
    else {
      sr::id_visitor siv;
      siv._([&](sr::ssa_id *si) {
        id_old_str = si->get_id()->to_string();
      });
      siv._default([&](id *di) {
        /*
         * Todo: Can this program point be reached?
         */
        id_old_str = di->to_string();
      });
      lhs_id_wrapped->accept(siv);
    }
    Expr id_old_exp = id_by_string(id_old_str);

    struct slice {
      Expr e;
      unsigned high;
      unsigned low;
      slice(Expr e, unsigned high, unsigned low) :
          e(e), high(high), low(low) {
      }
    };

    auto concat = [&](vector<slice> slices) {
      vector<Expr> exprs;
      for(auto &slice : slices) {
        auto extract_upper = BitVectorExtract(slice.high, slice.low);
        exprs.push_back(man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(extract_upper), slice.e));
      }
      rhs_conc = man.mkExpr(kind::BITVECTOR_CONCAT, exprs);
    };

    if(offset == 0)
      concat({slice(id_old_exp, 63, ass_size), slice(rhs, 0, ass_size - 1)});
    else if(offset + ass_size == 64)
      concat({slice(rhs, 63, offset), slice(id_old_exp, offset - 1, 0)});
    else
      concat({ slice(id_old_exp, 63, ass_size + offset), slice(rhs, ass_size - 1, 0), slice(id_old_exp, offset - 1, 0)});
  }
  Expr ass = man.mkExpr(kind::EQUAL, rhs_conc, lhs);
  sub_exprs.push_back(ass);
}


CVC4::Expr analysis::smt_builder::build(gdsl::rreil::statement *s, std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs) {
  this->defs = defs;
  s->accept(*this);
  return pop();
}
