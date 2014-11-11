/*
 * smt_builder.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/ismt/smt_builder.h>
#include <summy/tools/rreil_util.h>
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

  int_t ass_size = rreil_prop::size_of_assign(a);
  int_t offset = a->get_lhs()->get_offset();

  Expr rhs_conc;
  if(ass_size == 0 || ass_size == 64) rhs_conc = rhs;
  else {
    if(offset == 0) {
      shared_ptr<id> lhs_id_wrapped(a->get_lhs()->get_id(), [&](void *x) {});
      auto def_it = defs->get_elements().find(lhs_id_wrapped);
      string id_old_str;
      if(def_it != defs->get_elements().end()) id_old_str = def_it->first->to_string();
      else throw string("blah");
      Expr id_old_exp = id_by_string(id_old_str);

      auto extract_lower = BitVectorExtract(0, ass_size - 1);
      Expr ass_lower = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(extract_lower), rhs);

      auto extract_upper = BitVectorExtract(ass_size, 63);
      Expr ass_upper = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(extract_upper), id_old_exp);

      rhs_conc = man.mkExpr(kind::BITVECTOR_CONCAT, ass_upper, ass_lower);
    } else if(offset + ass_size == 64) {
      shared_ptr<id> lhs_id_wrapped(a->get_lhs()->get_id(), [&](void *x) {});
      auto def_it = defs->get_elements().find(lhs_id_wrapped);
      string id_old_str;
      if(def_it != defs->get_elements().end()) id_old_str = def_it->first->to_string();
      else throw string("blah");
      Expr id_old_exp = id_by_string(id_old_str);

      auto extract_lower = BitVectorExtract(offset - 1, 0);
      Expr ass_lower = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(extract_lower), id_old_exp);

      auto extract_upper = BitVectorExtract(offset, 63);
      Expr ass_upper = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(extract_upper), rhs);



      rhs_conc = man.mkExpr(kind::BITVECTOR_CONCAT, ass_upper, ass_lower);
    } else {
      shared_ptr<id> lhs_id_wrapped(a->get_lhs()->get_id(), [&](void *x) {});
      auto def_it = defs->get_elements().find(lhs_id_wrapped);
      string id_old_str;
      if(def_it != defs->get_elements().end()) id_old_str = def_it->first->to_string();
      else throw string("blah");
      Expr id_old_exp = id_by_string(id_old_str);

      auto extract_lower = BitVectorExtract(offset - 1, 0);
      Expr ass_lower = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(extract_lower), id_old_exp);

      auto extract_middle = BitVectorExtract(ass_size - 1, 0);
      Expr ass_middle = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(extract_middle), rhs);

      auto extract_upper = BitVectorExtract(63, offset + ass_size);
      Expr ass_upper = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(extract_upper), id_old_exp);

      rhs_conc = man.mkExpr(kind::BITVECTOR_CONCAT, ass_upper, ass_middle, ass_lower);
    }
  }
  Expr ass = man.mkExpr(kind::EQUAL, rhs_conc, lhs);
  sub_exprs.push_back(ass);
}



CVC4::Expr analysis::smt_builder::build(gdsl::rreil::statement *s, std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs) {
  this->defs = defs;
  s->accept(*this);
  return pop();
}
