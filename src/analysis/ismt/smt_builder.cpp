/*
 * smt_builder.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/ismt/smt_builder.h>
#include <summy/rreil/copy_visitor.h>
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/ssa_id.h>
#include <summy/tools/rreil_util.h>
#include <cppgdsl/rreil/rreil.h>
#include <string>
#include <vector>
#include <cmath>

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

    auto def_it = rd_result.result[from]->get_elements().find(lhs_id_wrapped);
    size_t def_node = 0;
    if(def_it != rd_result.result[from]->get_elements().end()) def_node = def_it->second;
    sr::copy_visitor civ;
    civ._([&](gdsl::rreil::id *inner, int_t rev) {
      return inner;
    });
    lhs_id_wrapped->accept(civ);
    id *id_old = def_node > 0 ? new sr::ssa_id(civ.get_id(), def_node) : civ.get_id();
    Expr id_old_exp = context.var(id_old->to_string());
    delete id_old;

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

    if(offset == 0) concat( { slice(id_old_exp, 63, size), slice(rhs, size - 1, 0) });
    else if(offset + size == 64) concat( { slice(rhs, 63, offset), slice(id_old_exp, offset - 1, 0) });
    else concat( { slice(id_old_exp, 63, size + offset), slice(rhs, size - 1, 0), slice(id_old_exp, offset - 1, 0) });
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

CVC4::Expr analysis::smt_builder::build(gdsl::rreil::statement *s) {
  s->accept(*this);
  return pop();
}

CVC4::Expr analysis::smt_builder::build(cfg::phi_assign const *pa) {
  handle_assign(pa->get_size(), pa->get_lhs(), [&]() {
    pa->get_rhs()->accept(*this);
  });
  return pop();
}

CVC4::Expr analysis::smt_builder::enforce_aligned(size_t size, CVC4::Expr address) {
  auto &man = context.get_manager();
  size_t addr_low_real_sz = log2(size/8);
  Expr addr_low_real = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(addr_low_real_sz - 1, 0)), address);
  Expr addr_constr = man.mkExpr(kind::EQUAL, addr_low_real, man.mkConst(BitVector(addr_low_real_sz, (unsigned long int)0)));
  return addr_constr;
}

CVC4::Expr analysis::smt_builder::extract_lower_bit_addr(CVC4::Expr address) {
  auto &man = context.get_manager();
  Expr lower_addr_bits = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(2, 0)), address);
  Expr lower_extended = man.mkExpr(kind::BITVECTOR_ZERO_EXTEND, man.mkConst(BitVectorZeroExtend(64 - 3 - 3)),
      lower_addr_bits);
  Expr lower_bit_addr = man.mkExpr(kind::BITVECTOR_CONCAT, lower_extended, man.mkConst(BitVector(3, (unsigned long int)0)));
  return lower_bit_addr;
}

void analysis::smt_builder::visit(gdsl::rreil::load *l) {
  rhs = true;
  l->get_address()->accept(*this);
  Expr address = pop();
  rhs = false;
  l->get_lhs()->accept(*this);
  Expr lhs = pop();

  Expr memory = context.memory(rd_result.result[from]->get_memory_rev());

  auto &man = context.get_manager();
  Expr addr_high = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(63, 3)), address);
  Expr drefed = man.mkExpr(kind::SELECT, memory, addr_high);

  if(l->get_size() < 64) {
    Expr lower_bit_addr = extract_lower_bit_addr(address);
    drefed = man.mkExpr(kind::BITVECTOR_LSHR, drefed, lower_bit_addr);
  } else if(l->get_size() > 64) throw string("Invalid size");

  Expr rhs_conc = concat_rhs(l->get_lhs()->get_id(), l->get_size(), l->get_lhs()->get_offset(), drefed);
  Expr load = man.mkExpr(kind::EQUAL, rhs_conc, lhs);

  Expr all = l->get_size() > 8 ? man.mkExpr(kind::AND, enforce_aligned(l->get_size(), address), load) : load;
  sub_exprs.push_back(all);
}

void analysis::smt_builder::visit(gdsl::rreil::store *s) {
  rhs = true;
  s->get_address()->accept(*this);
  Expr address = pop();
  s->get_rhs()->accept(*this);
  Expr rhs = pop();

  Expr memory_before = context.memory(rd_result.result[from]->get_memory_rev());
  Expr memory_after = context.memory(rd_result.result[to]->get_memory_rev());

  auto &man = context.get_manager();
  Expr addr_high = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(63, 3)), address);

  Expr mem_new;
  if(s->get_size() == 64) {
    mem_new = rhs;
  } else if(s->get_size() < 64) {
    Expr lower_bit_addr = extract_lower_bit_addr(address);

    Expr mask = man.mkConst(BitVector(64, (unsigned long int)((1 << s->get_size()) - 1)));
    mask = man.mkExpr(kind::BITVECTOR_SHL, mask, lower_bit_addr);
    mask = man.mkExpr(kind::BITVECTOR_NOT, mask);

    Expr mem_old = man.mkExpr(kind::SELECT, memory_before, addr_high);
    Expr mem_old_masked = man.mkExpr(kind::BITVECTOR_AND, mem_old, mask);

//        cout << mem_old_masked << endl;
//        context.get_smtEngine().checkSat(mem_old_masked);
//        cout << ":-)";

    Expr rhs_shifted = man.mkExpr(kind::BITVECTOR_SHL, rhs, lower_bit_addr);
    mem_new = man.mkExpr(kind::BITVECTOR_OR, mem_old_masked, rhs_shifted);
  } else throw string("Invalid size");
  Expr mem_stored = man.mkExpr(kind::STORE, memory_before, addr_high, mem_new);

  Expr store = man.mkExpr(kind::EQUAL, memory_after, mem_stored);

  Expr all = s->get_size() > 8 ? man.mkExpr(kind::AND, enforce_aligned(s->get_size(), address), store) : store;
  sub_exprs.push_back(all);
}

void analysis::smt_builder::edge(size_t from, size_t to) {
  this->from = from;
  this->to = to;
}
