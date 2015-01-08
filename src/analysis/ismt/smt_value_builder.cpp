/*
 * smt_builder.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/ismt/smt_value_builder.h>
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

void smt_value_builder::_default(gdsl::rreil::id *i) {
  auto i_str = i->to_string();
  Expr i_exp = context.var(i_str);
  set_accumulator(i_exp);
}

void analysis::smt_value_builder::visit(gdsl::rreil::lin_binop *a) {
  auto &man = context.get_manager();
  a->get_opnd1()->accept(*this);
  Expr opnd1 = pop_accumulator();
  a->get_opnd2()->accept(*this);
  Expr opnd2 = pop_accumulator();
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
  set_accumulator(r);
}

void analysis::smt_value_builder::visit(gdsl::rreil::lin_imm *a) {
  auto &man = context.get_manager();
  Expr imm = man.mkConst(BitVector(current_size(), (unsigned long int)a->get_imm()));
  set_accumulator(imm);
}

void analysis::smt_value_builder::visit(gdsl::rreil::lin_scale *a) {
  auto &man = context.get_manager();
  Expr factor_bv = man.mkConst(BitVector(64, (unsigned long int)a->get_const()));
  a->get_opnd()->accept(*this);
  Expr opnd = pop_accumulator();
  Expr r = man.mkExpr(kind::BITVECTOR_MULT, factor_bv, opnd);
  set_accumulator(r);
}

void analysis::smt_value_builder::visit(gdsl::rreil::expr_cmp *ec) {
  ec->get_opnd1()->accept(*this);
  Expr opnd1 = pop_accumulator();
  ec->get_opnd2()->accept(*this);
  Expr opnd2 = pop_accumulator();

  auto &man = context.get_manager();
  Expr result;
  switch(ec->get_op()) {
    case CMP_EQ: {
      result = man.mkExpr(kind::BITVECTOR_COMP, opnd1, opnd2);
      break;
    }
    case CMP_NEQ: {
      result = man.mkExpr(kind::BITVECTOR_NOT, man.mkExpr(kind::BITVECTOR_COMP, opnd1, opnd2));
      break;
    }
    case CMP_LES: {
      result = man.mkExpr(kind::BITVECTOR_SLE, opnd1, opnd2);
      break;
    }
    case CMP_LEU: {
      result = man.mkExpr(kind::BITVECTOR_ULE, opnd1, opnd2);
      break;
    }
    case CMP_LTS: {
      result = man.mkExpr(kind::BITVECTOR_SLT, opnd1, opnd2);
      break;
    }
    case CMP_LTU: {
      result = man.mkExpr(kind::BITVECTOR_ULT, opnd1, opnd2);
      break;
    }
    default: {
      throw string("Invalid comparison");
    }
  }


  if(result.getType() == man.booleanType()) result = man.mkExpr(kind::ITE, result,
      man.mkConst(BitVector(1, (unsigned long int)1)), man.mkConst(BitVector(1, (unsigned long int)0)));
  replace_size(1);
  set_accumulator(result);
}

void analysis::smt_value_builder::visit(gdsl::rreil::arbitrary *ab) {
  /*
   * Todo: Arbitrary constructor?
   */
  auto &man = context.get_manager();
  Expr i_exp = man.mkVar("arbitrary", man.mkBitVectorType(current_size()));
  set_accumulator(i_exp);
}


void analysis::smt_value_builder::visit(gdsl::rreil::expr_binop *eb) {
  eb->get_opnd1()->accept(*this);
  Expr opnd1 = pop_accumulator();
  eb->get_opnd2()->accept(*this);
  Expr opnd2 = pop_accumulator();

  ::CVC4::kind::Kind_t k;
  switch(eb->get_op()) {
    case BIN_MUL: {
      k = kind::BITVECTOR_MULT;
      break;
    }
    case BIN_DIV: {
      k = kind::BITVECTOR_UDIV;
      break;
    }
    case BIN_DIVS: {
      k = kind::BITVECTOR_SDIV;
      break;
    }
    case BIN_MOD: {
      k = kind::BITVECTOR_UREM;
      break;
    }
    case BIN_MODS: {
      k = kind::BITVECTOR_SREM;
      break;
    }
    case BIN_SHL: {
      k = kind::BITVECTOR_SHL;
      break;
    }
    case BIN_SHR: {
      k = kind::BITVECTOR_LSHR;
      break;
    }
    case BIN_SHRS: {
      k = kind::BITVECTOR_ASHR;
      break;
    }
    case BIN_AND: {
      k = kind::BITVECTOR_AND;
      break;
    }
    case BIN_OR: {
      k = kind::BITVECTOR_OR;
      break;
    }
    case BIN_XOR: {
      k = kind::BITVECTOR_XOR;
      break;
    }
    default: {
      throw string("Invalid expression");
    }
  }
  auto &man = context.get_manager();
  Expr result = man.mkExpr(k, opnd1, opnd2);
  set_accumulator(result);
}

void analysis::smt_value_builder::visit(gdsl::rreil::expr_ext *ext) {
  size_t to_size = current_size();
  push_size(ext->get_fromsize());
  ext->get_opnd()->accept(*this);
  Expr opnd = pop_accumulator();
  pop_size();

  assert(to_size > 0);

  auto &man = context.get_manager();
  Expr r;
  if(ext->get_fromsize() > to_size) r = man.mkExpr(kind::BITVECTOR_EXTRACT,
      man.mkConst(BitVectorExtract(to_size - 1, 0)), opnd);
  else switch(ext->get_op()) {
    case EXT_ZX: {
      r = man.mkExpr(kind::BITVECTOR_ZERO_EXTEND, man.mkConst(BitVectorZeroExtend(to_size - ext->get_fromsize())),
          opnd);
      break;
    }
    case EXT_SX: {
      r = man.mkExpr(kind::BITVECTOR_SIGN_EXTEND, man.mkConst(BitVectorSignExtend(to_size - ext->get_fromsize())),
          opnd);
      break;
    }
    default: {
      throw string("Invalid extension");
    }
  }
  set_accumulator(r);
  ;
}

void analysis::smt_value_builder::visit(gdsl::rreil::address *addr) {
  push_size(addr->get_size());
  addr->get_lin()->accept(*this);
  pop_size();
}

CVC4::Expr analysis::smt_value_builder::get_id_old_exp(gdsl::rreil::id *id, size_t def_node) {
  return context.var(id->to_string());
}

void analysis::smt_value_builder::handle_assign(size_t size, gdsl::rreil::variable *lhs_gr, std::function<void()> rhs_accept) {
  /*
   * Todo: what if size == 0?
   */
//    base::visit(a);
  auto &man = context.get_manager();
  sizes.push_back(size);
  rhs_accept();
  Expr rhs = pop_accumulator();
  lhs_gr->get_id()->accept(*this);
  Expr lhs = pop_accumulator();
  size = pop_size();

//  int_t ass_size = rreil_prop::size_of_assign(a);
  int_t ass_size = size;
  int_t offset = lhs_gr->get_offset();

  Expr rhs_conc = concat_rhs(lhs_gr->get_id(), ass_size, offset, rhs);

  Expr ass = man.mkExpr(kind::EQUAL, rhs_conc, lhs);
  set_accumulator(ass);

//    cout << ass << endl;
////    context.get_smtEngine().checkSat(man.mkExpr(kind::EQUAL, result, result));
//    context.get_smtEngine().checkSat(ass);
//    cout << ":-)" << endl;
}

void smt_value_builder::visit(gdsl::rreil::assign *a) {
  handle_assign(rreil_prop::size_of_assign(a), a->get_lhs(), [&]() {
    a->get_rhs()->accept(*this);
  });
}

CVC4::Expr analysis::smt_value_builder::enforce_aligned(size_t size, CVC4::Expr address) {
  auto &man = context.get_manager();
//  size_t addr_low_real_sz = log2(size/8);
//  Expr addr_low_real = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(addr_low_real_sz - 1, 0)), address);
//  Expr addr_constr = man.mkExpr(kind::EQUAL, addr_low_real, man.mkConst(BitVector(addr_low_real_sz, (unsigned long int)0)));
  Expr addr_constr = man.mkConst(true);
  return addr_constr;
}

CVC4::Expr analysis::smt_value_builder::extract_lower_bit_addr(CVC4::Expr address) {
  auto &man = context.get_manager();
  Expr lower_addr_bits = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(2, 0)), address);
  Expr lower_extended = man.mkExpr(kind::BITVECTOR_ZERO_EXTEND, man.mkConst(BitVectorZeroExtend(64 - 3 - 3)),
      lower_addr_bits);
  Expr lower_bit_addr = man.mkExpr(kind::BITVECTOR_CONCAT, lower_extended, man.mkConst(BitVector(3, (unsigned long int)0)));
  return lower_bit_addr;
}

CVC4::Expr analysis::smt_value_builder::load_memory(CVC4::Expr memory, size_t size, CVC4::Expr address) {
  auto &man = context.get_manager();
  Expr addr_high = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(63, 3)), address);
  Expr drefed = man.mkExpr(kind::SELECT, memory, addr_high);

  if(size < 64) {
    Expr lower_bit_addr = extract_lower_bit_addr(address);
    drefed = man.mkExpr(kind::BITVECTOR_LSHR, drefed, lower_bit_addr);
    drefed = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(size - 1, 0)), drefed);
  } else if(size > 64) throw string("Invalid size");

  return drefed;
}

void analysis::smt_value_builder::visit(gdsl::rreil::load *l) {
  push_size(l->get_size());
  l->get_address()->accept(*this);
  Expr address = pop_accumulator();
  l->get_lhs()->get_id()->accept(*this);
  Expr lhs = pop_accumulator();
  pop_size();

  Expr memory = context.memory(rd_result.result[from]->get_memory_rev());
  Expr drefed = load_memory(memory, l->get_size(), address);

  auto &man = context.get_manager();
  Expr rhs_conc = concat_rhs(l->get_lhs()->get_id(), l->get_size(), l->get_lhs()->get_offset(), drefed);
  Expr load = man.mkExpr(kind::EQUAL, rhs_conc, lhs);

  Expr all = l->get_size() > 8 ? man.mkExpr(kind::AND, enforce_aligned(l->get_size(), address), load) : load;
  set_accumulator(all);
}

CVC4::Expr analysis::smt_value_builder::store_memory(CVC4::Expr memory_before, size_t size, CVC4::Expr address, CVC4::Expr value) {
  auto &man = context.get_manager();
  Expr addr_high = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(63, 3)), address);

  assert(size <= 64);
  Expr value_ext = size < 64 ? man.mkExpr(kind::BITVECTOR_ZERO_EXTEND, man.mkConst(BitVectorZeroExtend(64 - size)), value) : value;

  Expr mem_new;
  if(size == 64) {
    mem_new = value_ext;
  } else if(size < 64) {
    Expr lower_bit_addr = extract_lower_bit_addr(address);

    Expr mask = man.mkConst(BitVector(64, (unsigned long int)((1 << size) - 1)));
    mask = man.mkExpr(kind::BITVECTOR_SHL, mask, lower_bit_addr);
    mask = man.mkExpr(kind::BITVECTOR_NOT, mask);

    Expr mem_old = man.mkExpr(kind::SELECT, memory_before, addr_high);
    Expr mem_old_masked = man.mkExpr(kind::BITVECTOR_AND, mem_old, mask);

    Expr rhs_shifted = man.mkExpr(kind::BITVECTOR_SHL, value_ext, lower_bit_addr);
    mem_new = man.mkExpr(kind::BITVECTOR_OR, mem_old_masked, rhs_shifted);
  } else throw string("Invalid size");
  Expr mem_stored = man.mkExpr(kind::STORE, memory_before, addr_high, mem_new);

  return  mem_stored;
}

void analysis::smt_value_builder::visit(gdsl::rreil::store *s) {
  push_size(s->get_size());
  s->get_address()->accept(*this);
  Expr address = pop_accumulator();
  s->get_rhs()->accept(*this);
  Expr rhs = pop_accumulator();
  size_t size = pop_size();

  Expr memory_before = context.memory(rd_result.result[from]->get_memory_rev());

  Expr mem_stored = store_memory(memory_before, size, address, rhs);

  Expr memory_after = context.memory(rd_result.result[to]->get_memory_rev());
  auto &man = context.get_manager();
  Expr store = man.mkExpr(kind::EQUAL, memory_after, mem_stored);

  Expr all = size > 8 ? man.mkExpr(kind::AND, enforce_aligned(size, address), store) : store;
  set_accumulator(all);
}

CVC4::Expr analysis::smt_value_builder::build(gdsl::rreil::statement *s) {
  s->accept(*this);
  return pop_accumulator();
}

CVC4::Expr analysis::smt_value_builder::build(gdsl::rreil::address *addr) {
  addr->accept(*this);
  return pop_accumulator();
}

CVC4::Expr analysis::smt_value_builder::build(cfg::phi_assign const *pa) {
  handle_assign(pa->get_size(), pa->get_lhs(), [&]() {
    pa->get_rhs()->accept(*this);
  });
  return pop_accumulator();
}

CVC4::Expr analysis::smt_value_builder::build(cfg::phi_memory const& pm) {
  auto &man = context.get_manager();
  return man.mkExpr(kind::EQUAL, context.memory(pm.from), context.memory(pm.to));
}

void analysis::smt_value_builder::edge(size_t from, size_t to) {
  this->from = from;
  this->to = to;
}
