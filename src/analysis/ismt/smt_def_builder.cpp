/*
 * smt_def_builder.cpp
 *
 *  Created on: Dec 15, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/ismt/smt_def_builder.h>
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

CVC4::Expr analysis::smt_def_builder::get_id_old_exp(gdsl::rreil::id *id, size_t def_node) {
  return id_at_rev(id, def_node);
}

CVC4::Expr analysis::smt_def_builder::defined_boolbv(CVC4::Expr a) {
  size_t size = current_size();

  Expr fully = manager.mkConst(BitVector(size, (unsigned long int)(-1)));
  Expr defined = manager.mkExpr(kind::BITVECTOR_COMP, a, fully);

  return defined;
}

CVC4::Expr analysis::smt_def_builder::defined(CVC4::Expr a) {
  size_t size = current_size();

  Expr defined = defined_boolbv(a);
  return manager.mkExpr(kind::BITVECTOR_REPEAT, manager.mkConst(BitVectorRepeat(size)), defined);
}

CVC4::Expr analysis::smt_def_builder::defined_boolbv(CVC4::Expr a, CVC4::Expr b) {
  Expr anded = manager.mkExpr(kind::BITVECTOR_AND, a, b);
  return defined_boolbv(anded);
}

CVC4::Expr analysis::smt_def_builder::defined(CVC4::Expr a, CVC4::Expr b) {
  Expr anded = manager.mkExpr(kind::BITVECTOR_AND, a, b);
  return defined(anded);
}

CVC4::Expr analysis::smt_def_builder::var(std::string name) {
  return context.var(name);
}

CVC4::Expr analysis::smt_def_builder::var_def(std::string name) {
  return context.var(name + "_def");
}

CVC4::Expr analysis::smt_def_builder::id_at_rev(gdsl::rreil::id *i, size_t rev) {
  Expr result;
  if(!rev) {
    Expr zero = manager.mkConst(BitVector(64, (unsigned long int)(0)));
    result = zero;
  } else {
    auto i_str = i->to_string();
    result = var_def(i_str);
  }
  return result;
}

void smt_def_builder::_default(gdsl::rreil::id *i) {
  set_accumulator(id_at_rev(i, 0));
}

void analysis::smt_def_builder::visit(summy::rreil::ssa_id *si) {
  set_accumulator(id_at_rev(si, si->get_version()));
}

void analysis::smt_def_builder::visit(gdsl::rreil::lin_binop *a) {
  a->get_opnd1()->accept(*this);
  Expr opnd1 = pop_accumulator();
  a->get_opnd2()->accept(*this);
  Expr opnd2 = pop_accumulator();
  Expr r;
  switch(a->get_op()) {
    case BIN_LIN_ADD: {
      r = defined(opnd1, opnd2);
      break;
    }
    case BIN_LIN_SUB: {
      r = defined(opnd1, opnd2);
      break;
    }
  }
  set_accumulator(r);
}

void analysis::smt_def_builder::visit(gdsl::rreil::lin_imm *a) {
  Expr imm = manager.mkConst(BitVector(current_size(), (unsigned long int)(-1)));
  set_accumulator(imm);
}

void analysis::smt_def_builder::visit(gdsl::rreil::lin_scale *a) {
  a->get_opnd()->accept(*this);
}

void analysis::smt_def_builder::visit(gdsl::rreil::expr_cmp *ec) {
  ec->get_opnd1()->accept(*this);
  Expr opnd1 = pop_accumulator();
  ec->get_opnd2()->accept(*this);
  Expr opnd2 = pop_accumulator();

  Expr result;
  switch(ec->get_op()) {
    case CMP_EQ: {
      result = defined_boolbv(opnd1, opnd2);
      break;
    }
    case CMP_NEQ: {
      result = defined_boolbv(opnd1, opnd2);
      break;
    }
    case CMP_LES: {
      result = defined_boolbv(opnd1, opnd2);
      break;
    }
    case CMP_LEU: {
      result = defined_boolbv(opnd1, opnd2);
      break;
    }
    case CMP_LTS: {
      result = defined_boolbv(opnd1, opnd2);;
      break;
    }
    case CMP_LTU: {
      result = defined_boolbv(opnd1, opnd2);
      break;
    }
    default: {
      throw string("Invalid comparison");
    }
  }


//  if(result.getType() == man.booleanType()) result = man.mkExpr(kind::ITE, result,
//      man.mkConst(BitVector(1, (unsigned long int)1)), man.mkConst(BitVector(1, (unsigned long int)0)));
  replace_size(1);
  set_accumulator(result);
}

void analysis::smt_def_builder::visit(gdsl::rreil::arbitrary *ab) {
  Expr i_exp = manager.mkConst(BitVector(current_size(), (unsigned long int)(0)));
  set_accumulator(i_exp);
}

void analysis::smt_def_builder::visit(gdsl::rreil::expr_binop *eb) {
  eb->get_opnd1()->accept(*this);
  Expr opnd1 = pop_accumulator();
  eb->get_opnd2()->accept(*this);
  Expr opnd2 = pop_accumulator();

  Expr result;

//  auto if_def = [&](Expr cond, Expr then) {
//    Expr e_def = defined(e);
//  };

  switch(eb->get_op()) {
    case BIN_MUL: {
      result = defined(opnd1, opnd2);
      break;
    }
    case BIN_DIV: {
      result = defined(opnd1, opnd2);
      break;
    }
    case BIN_DIVS: {
      result = defined(opnd1, opnd2);
      break;
    }
    case BIN_MOD: {
      result = defined(opnd1, opnd2);
      break;
    }
    case BIN_MODS: {
      result = defined(opnd1, opnd2);
      break;
    }
    case BIN_SHL: {
      result = defined(opnd1, opnd2);;
      break;
    }
    case BIN_SHR: {
      result = defined(opnd1, opnd2);;
      break;
    }
    case BIN_SHRS: {
      result = defined(opnd1, opnd2);
      break;
    }
    case BIN_AND: {
      result = manager.mkExpr(kind::BITVECTOR_AND, opnd1, opnd2);
      break;
    }
    case BIN_OR: {
      result = manager.mkExpr(kind::BITVECTOR_AND, opnd1, opnd2);
      break;
    }
    case BIN_XOR: {
      result = manager.mkExpr(kind::BITVECTOR_AND, opnd1, opnd2);
      break;
    }
    default: {
      throw string("Invalid expression");
    }
  }
  set_accumulator(result);
}

void analysis::smt_def_builder::visit(gdsl::rreil::expr_ext *ext) {
  size_t tosize = current_size();
  size_t fromsize = ext->get_fromsize();
  size_t op_size = fromsize < tosize ? fromsize : tosize;

  push_size(op_size);
  ext->get_opnd()->accept(*this);
  Expr opnd = pop_accumulator();
  pop_size();

  assert(tosize > 0);

  Expr result;
  if(fromsize > tosize) result = manager.mkExpr(kind::BITVECTOR_EXTRACT,
      manager.mkConst(BitVectorExtract(tosize - 1, 0)), opnd);
  else if(tosize == fromsize)
    result = opnd;
  else switch(ext->get_op()) {
    case EXT_ZX: {
      Expr upper_bits = manager.mkConst(BitVector(tosize - fromsize, (unsigned long int)(-1)));
      result = manager.mkExpr(kind::BITVECTOR_CONCAT, upper_bits, opnd);
      break;
    }
    case EXT_SX: {
      result = manager.mkExpr(kind::BITVECTOR_SIGN_EXTEND, manager.mkConst(BitVectorSignExtend(tosize - fromsize)),
          opnd);
      break;
    }
    default: {
      throw string("Invalid extension");
    }
  }

  set_accumulator(result);
}

void analysis::smt_def_builder::visit(gdsl::rreil::load *l) {
  push_size(l->get_size());
//  l->get_address()->accept(*this);
//  Expr address = pop_accumulator();
  smt_value_builder sb(context, rd_result);
  Expr address = sb.build(l->get_address());

  l->get_lhs()->get_id()->accept(*this);
  Expr lhs = pop_accumulator();
  pop_size();

  size_t mem_rev = rd_result.result[from]->get_memory_rev();
  Expr memory = context.memory_def(mem_rev);

  Expr drefed = sb.load_memory(memory, l->get_size(), address);

  Expr rhs_conc = concat_rhs(l->get_lhs()->get_id(), l->get_size(), l->get_lhs()->get_offset(), drefed);
  Expr load = manager.mkExpr(kind::EQUAL, rhs_conc, lhs);

  Expr memory_ini = context.memory_def(0);
  Expr addr_high = manager.mkExpr(kind::BITVECTOR_EXTRACT, manager.mkConst(BitVectorExtract(63, 3)), address);
  Expr drefed_ini = manager.mkExpr(kind::SELECT, memory_ini, addr_high);
  Expr initialization = manager.mkExpr(kind::EQUAL, drefed_ini, manager.mkConst(BitVector(64, (unsigned long int)(0))));

  Expr all_plus_init = manager.mkExpr(kind::AND, load, initialization);

//  cout << all_plus_init << endl;

  set_accumulator(all_plus_init);
}

void analysis::smt_def_builder::visit(gdsl::rreil::store *s) {
  push_size(s->get_size());
  smt_value_builder sb(context, rd_result);
  Expr address = sb.build(s->get_address());

  s->get_rhs()->accept(*this);
  Expr rhs = pop_accumulator();
  size_t size = pop_size();

  Expr memory_before = context.memory_def(rd_result.result[from]->get_memory_rev());

  Expr mem_stored = sb.store_memory(memory_before, size, address, rhs);

  Expr memory_after = context.memory_def(rd_result.result[to]->get_memory_rev());
  Expr store = manager.mkExpr(kind::EQUAL, memory_after, mem_stored);

  set_accumulator(store);
}

CVC4::Expr analysis::smt_def_builder::build_target(gdsl::rreil::address *addr) {
  addr->accept(*this);
  push_size(addr->get_size());
  Expr result = manager.mkExpr(kind::EQUAL, defined_boolbv(pop_accumulator()), manager.mkConst(BitVector(1, (unsigned long int)(1))));
  pop_size();
  return result;
}
