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
  if(sub_exprs.empty()) throw string(":/");
  Expr r = sub_exprs.back();
  sub_exprs.pop_back();
  return r;
}

void smt_builder::_default(gdsl::rreil::id *i) {
  auto i_str = i->to_string();
  Expr i_exp = context.var(i_str);
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

CVC4::Expr analysis::smt_builder::concat_rhs(id *lhs_id, size_t size, size_t offset, Expr rhs) {
  auto &man = context.get_manager();
  Expr rhs_conc;
  if(size == 0 || size == 64) rhs_conc = rhs;
  else {
    shared_ptr<id> lhs_id_wrapped(lhs_id, [&](void *x) {});
    auto def_it = defs_before->get_elements().find(lhs_id_wrapped);
    string id_old_str;
    if(def_it != defs_before->get_elements().end()) id_old_str = def_it->first->to_string();
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
    Expr id_old_exp = context.var(id_old_str);

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

  if(offset == 0) concat({ slice(id_old_exp, 63, size), slice(rhs, 0, size - 1) });
  else if(offset + size == 64) concat( { slice(rhs, 63, offset), slice(id_old_exp, offset - 1, 0) });
  else concat(
      { slice(id_old_exp, 63, size + offset), slice(rhs, size - 1, 0), slice(id_old_exp, offset - 1, 0) });
  }
  return rhs_conc;
}

void analysis::smt_builder::handle_assign(size_t size, gdsl::rreil::variable *lhs_gr, std::function<void()> rhs_accept) {
//    base::visit(a);
  auto &man = context.get_manager();
  rhs = true;
  rhs_accept();
  rhs = false;
  Expr rhs = pop();
  lhs_gr->accept(*this);
  Expr lhs = pop();

//  int_t ass_size = rreil_prop::size_of_assign(a);
  int_t ass_size = size;
  int_t offset = lhs_gr->get_offset();

  Expr rhs_conc = concat_rhs(lhs_gr->get_id(), ass_size, offset, rhs);

  Expr ass = man.mkExpr(kind::EQUAL, rhs_conc, lhs);
  sub_exprs.push_back(ass);
}


void smt_builder::visit(gdsl::rreil::assign *a) {
  handle_assign(rreil_prop::size_of_assign(a), a->get_lhs(), [&]() {
    a->get_rhs()->accept(*this);
  });
}

CVC4::Expr analysis::smt_builder::build(gdsl::rreil::statement *s,
    std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs_before,
    std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs_after) {
  this->defs_before = defs_before;
  this->defs_after = defs_after;
  s->accept(*this);
  return pop();
}

CVC4::Expr analysis::smt_builder::build(cfg::phi_assign const *pa,
    std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs_before,
    std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs_after) {
  this->defs_before = defs_before;
  this->defs_after = defs_after;
  handle_assign(pa->get_size(), pa->get_lhs(), [&]() {
    pa->get_rhs()->accept(*this);
  });
  return pop();
}

void analysis::smt_builder::visit(gdsl::rreil::load *l) {
  rhs = true;
  l->get_address()->accept(*this);
  Expr address = pop();
  rhs = false;
  l->get_lhs()->accept(*this);
  Expr lhs = pop();

  Expr memory = context.memory(defs_before->get_memory_rev());

  auto &man = context.get_manager();
  Expr higher_addr_bits = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(63, 8)), address);
  Expr drefed = man.mkExpr(kind::SELECT, memory, higher_addr_bits, address);

  if(l->get_size() < 64) {
    Expr lower_addr_bits = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(7, 0)), address);
    Expr lower_extended = man.mkExpr(kind::BITVECTOR_ZERO_EXTEND, man.mkConst(BitVectorZeroExtend(64 - 3)),
        lower_addr_bits);
//    Expr lower_bit_addr = man.mkExpr(kind::BITVECTOR_SHL, lower_extended, man.mkConst(BitVector(64, 3)));
    Expr lower_bit_addr = man.mkExpr(kind::BITVECTOR_CONCAT, lower_extended, man.mkConst(BitVector(3, (unsigned long int)0)));
    drefed = man.mkExpr(kind::BITVECTOR_LSHR, drefed, lower_bit_addr);
  } else if(l->get_size() > 64)
    throw string("Invalid size");

  Expr rhs_conc = concat_rhs(l->get_lhs()->get_id(), l->get_size(), l->get_lhs()->get_offset(), drefed);

  Expr load = man.mkExpr(kind::EQUAL, rhs_conc, lhs);
  sub_exprs.push_back(load);
}

void analysis::smt_builder::visit(gdsl::rreil::store *s) {
  rhs = true;
  s->get_address()->accept(*this);
  Expr address = pop();
  s->get_rhs()->accept(*this);
  Expr rhs = pop();

  Expr memory_before = context.memory(defs_before->get_memory_rev());
  Expr memory_after = context.memory(defs_after->get_memory_rev());

  auto &man = context.get_manager();
  if(s->get_size() == 64) {
    //below
  } else
    throw 42;

//  Expr drefed = man.mkExpr(kind::SELECT, memory_before, address);
  Expr mem_stored = man.mkExpr(kind::STORE, memory_before, address, rhs);

  Expr store = man.mkExpr(kind::EQUAL, memory_after, mem_stored);
  sub_exprs.push_back(store);
}
