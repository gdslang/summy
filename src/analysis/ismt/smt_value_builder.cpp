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

CVC4::Expr analysis::smt_value_builder::get_id_old_exp(gdsl::rreil::id const *id, size_t def_node) {
  return context.var(id->to_string());
}

CVC4::Expr analysis::smt_value_builder::enforce_aligned(size_t size, CVC4::Expr address) {
//  size_t addr_low_real_sz = log2(size/8);
//  Expr addr_low_real = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(addr_low_real_sz - 1, 0)), address);
//  Expr addr_constr = man.mkExpr(kind::EQUAL, addr_low_real, man.mkConst(BitVector(addr_low_real_sz, (unsigned long int)0)));
  Expr addr_constr = manager.mkConst(true);
  return addr_constr;
}

void smt_value_builder::_default(gdsl::rreil::id const *i) {
  auto i_str = i->to_string();
  Expr i_exp = context.var(i_str);
  set_accumulator(i_exp);
}

void analysis::smt_value_builder::visit(gdsl::rreil::lin_binop const *a) {
  a->get_lhs().accept(*this);
  Expr opnd1 = pop_accumulator();
  a->get_rhs().accept(*this);
  Expr opnd2 = pop_accumulator();
  Expr r;
  switch(a->get_op()) {
    case BIN_LIN_ADD: {
      r = manager.mkExpr(kind::BITVECTOR_PLUS, opnd1, opnd2);
      break;
    }
    case BIN_LIN_SUB: {
      r = manager.mkExpr(kind::BITVECTOR_SUB, opnd1, opnd2);
      break;
    }
  }
  set_accumulator(r);
}

void analysis::smt_value_builder::visit(gdsl::rreil::lin_imm const *a) {
  Expr imm = manager.mkConst(BitVector(current_size(), (unsigned long int)a->get_imm()));
  set_accumulator(imm);
}

void analysis::smt_value_builder::visit(gdsl::rreil::lin_scale const *a) {
  Expr factor_bv = manager.mkConst(BitVector(64, (unsigned long int)a->get_const()));
  a->get_operand().accept(*this);
  Expr opnd = pop_accumulator();
  Expr r = manager.mkExpr(kind::BITVECTOR_MULT, factor_bv, opnd);
  set_accumulator(r);
}

void analysis::smt_value_builder::visit(gdsl::rreil::expr_cmp const *ec) {
  ec->get_lhs().accept(*this);
  Expr opnd1 = pop_accumulator();
  ec->get_rhs().accept(*this);
  Expr opnd2 = pop_accumulator();

  Expr result;
  switch(ec->get_op()) {
    case CMP_EQ: {
      result = manager.mkExpr(kind::BITVECTOR_COMP, opnd1, opnd2);
      break;
    }
    case CMP_NEQ: {
      result = manager.mkExpr(kind::BITVECTOR_NOT, manager.mkExpr(kind::BITVECTOR_COMP, opnd1, opnd2));
      break;
    }
    case CMP_LES: {
      result = manager.mkExpr(kind::BITVECTOR_SLE, opnd1, opnd2);
      break;
    }
    case CMP_LEU: {
      result = manager.mkExpr(kind::BITVECTOR_ULE, opnd1, opnd2);
      break;
    }
    case CMP_LTS: {
      result = manager.mkExpr(kind::BITVECTOR_SLT, opnd1, opnd2);
      break;
    }
    case CMP_LTU: {
      result = manager.mkExpr(kind::BITVECTOR_ULT, opnd1, opnd2);
      break;
    }
    default: {
      throw string("Invalid comparison");
    }
  }


  if(result.getType() == manager.booleanType()) result = manager.mkExpr(kind::ITE, result,
      manager.mkConst(BitVector(1, (unsigned long int)1)), manager.mkConst(BitVector(1, (unsigned long int)0)));
  replace_size(1);
  set_accumulator(result);
}

void analysis::smt_value_builder::visit(gdsl::rreil::arbitrary const *ab) {
  /*
   * Todo: Arbitrary constructor?
   */
  Expr i_exp = manager.mkVar("arbitrary", manager.mkBitVectorType(current_size()));
  set_accumulator(i_exp);
}


void analysis::smt_value_builder::visit(gdsl::rreil::expr_binop const *eb) {
  eb->get_lhs().accept(*this);
  Expr opnd1 = pop_accumulator();
  eb->get_rhs().accept(*this);
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
  Expr result = manager.mkExpr(k, opnd1, opnd2);
  set_accumulator(result);
}

void analysis::smt_value_builder::visit(gdsl::rreil::expr_ext const *ext) {
  size_t tosize = current_size();
  size_t fromsize = ext->get_fromsize();
  size_t op_size = fromsize < tosize ? fromsize : tosize;

  push_size(op_size);
  ext->get_operand().accept(*this);
  Expr opnd = pop_accumulator();
  pop_size();

  assert(tosize > 0);

  Expr r;
  if(ext->get_fromsize() > tosize) r = manager.mkExpr(kind::BITVECTOR_EXTRACT,
      manager.mkConst(BitVectorExtract(tosize - 1, 0)), opnd);
  else switch(ext->get_op()) {
    case EXT_ZX: {
      r = manager.mkExpr(kind::BITVECTOR_ZERO_EXTEND, manager.mkConst(BitVectorZeroExtend(tosize - ext->get_fromsize())),
          opnd);
      break;
    }
    case EXT_SX: {
      r = manager.mkExpr(kind::BITVECTOR_SIGN_EXTEND, manager.mkConst(BitVectorSignExtend(tosize - ext->get_fromsize())),
          opnd);
      break;
    }
    default: {
      throw string("Invalid extension");
    }
  }
  set_accumulator(r);
}

void analysis::smt_value_builder::visit(gdsl::rreil::load const *l) {
  push_size(l->get_size());
  l->get_address().accept(*this);
  Expr address = pop_accumulator();
  l->get_lhs().get_id().accept(*this);
  Expr lhs = pop_accumulator();
  pop_size();

  Expr memory = context.memory(rd_result.result[from]->get_memory_rev());
  Expr drefed = load_memory(memory, l->get_size(), address);

  Expr rhs_conc = concat_rhs(&l->get_lhs().get_id(), l->get_size(), l->get_lhs().get_offset(), drefed);
  Expr load = manager.mkExpr(kind::EQUAL, rhs_conc, lhs);

  Expr all = l->get_size() > 8 ? manager.mkExpr(kind::AND, enforce_aligned(l->get_size(), address), load) : load;
  set_accumulator(all);
}

void analysis::smt_value_builder::visit(gdsl::rreil::store const *s) {
  push_size(s->get_size());
  s->get_address().accept(*this);
  Expr address = pop_accumulator();
  s->get_rhs().accept(*this);
  Expr rhs = pop_accumulator();
  size_t size = pop_size();

  Expr memory_before = context.memory(rd_result.result[from]->get_memory_rev());

  Expr mem_stored = store_memory(memory_before, size, address, rhs);

  Expr memory_after = context.memory(rd_result.result[to]->get_memory_rev());
  Expr store = manager.mkExpr(kind::EQUAL, memory_after, mem_stored);

  Expr all = size > 8 ? manager.mkExpr(kind::AND, enforce_aligned(size, address), store) : store;
  set_accumulator(all);
}

CVC4::Expr analysis::smt_value_builder::load_memory(CVC4::Expr memory, size_t size, CVC4::Expr address) {
  Expr addr_high = manager.mkExpr(kind::BITVECTOR_EXTRACT, manager.mkConst(BitVectorExtract(63, 3)), address);
  Expr drefed = manager.mkExpr(kind::SELECT, memory, addr_high);

  if(size < 64) {
    Expr lower_bit_addr = extract_lower_bit_addr(address);
    drefed = manager.mkExpr(kind::BITVECTOR_LSHR, drefed, lower_bit_addr);
    drefed = manager.mkExpr(kind::BITVECTOR_EXTRACT, manager.mkConst(BitVectorExtract(size - 1, 0)), drefed);
  } else if(size > 64) throw string("Invalid size");

  return drefed;
}

CVC4::Expr analysis::smt_value_builder::store_memory(CVC4::Expr memory_before, size_t size, CVC4::Expr address, CVC4::Expr value) {
  Expr addr_high = manager.mkExpr(kind::BITVECTOR_EXTRACT, manager.mkConst(BitVectorExtract(63, 3)), address);

  assert(size <= 64);
  Expr value_ext = size < 64 ? manager.mkExpr(kind::BITVECTOR_ZERO_EXTEND, manager.mkConst(BitVectorZeroExtend(64 - size)), value) : value;

  Expr mem_new;
  if(size == 64) {
    mem_new = value_ext;
  } else if(size < 64) {
    Expr lower_bit_addr = extract_lower_bit_addr(address);

    Expr mask = manager.mkConst(BitVector(64, (unsigned long int)((1 << size) - 1)));
    mask = manager.mkExpr(kind::BITVECTOR_SHL, mask, lower_bit_addr);
    mask = manager.mkExpr(kind::BITVECTOR_NOT, mask);

    Expr mem_old = manager.mkExpr(kind::SELECT, memory_before, addr_high);
    Expr mem_old_masked = manager.mkExpr(kind::BITVECTOR_AND, mem_old, mask);

    Expr rhs_shifted = manager.mkExpr(kind::BITVECTOR_SHL, value_ext, lower_bit_addr);
    mem_new = manager.mkExpr(kind::BITVECTOR_OR, mem_old_masked, rhs_shifted);
  } else throw string("Invalid size");
  Expr mem_stored = manager.mkExpr(kind::STORE, memory_before, addr_high, mem_new);

  return  mem_stored;
}

CVC4::Expr analysis::smt_value_builder::build(gdsl::rreil::address const *addr) {
  addr->accept(*this);
  return pop_accumulator();
}
