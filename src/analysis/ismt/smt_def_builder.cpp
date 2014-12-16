/*
 * smt_def_builder.cpp
 *
 *  Created on: Dec 15, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/ismt/smt_def_builder.h>
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

size_t analysis::smt_def_builder::current_size() {
  if(sizes.empty())
    throw string("Size unknown");
  return sizes.back();
}

size_t analysis::smt_def_builder::pop_size() {
  if(sizes.empty())
    throw string("Empty size stack");
  size_t back = sizes.back();
  sizes.pop_back();
  return back;
}

void analysis::smt_def_builder::push_size(size_t size) {
  sizes.push_back(size);
}

void analysis::smt_def_builder::replace_size(size_t size) {
  pop_size();
  push_size(size);
}

CVC4::Expr analysis::smt_def_builder::current_accumulator() {
  if(!accumulator_set) throw string(":/");
  return accumulator;
}

CVC4::Expr analysis::smt_def_builder::pop_accumulator() {
  if(!accumulator_set) throw string(":/");
  this->accumulator_set = false;
  return accumulator;
}

void analysis::smt_def_builder::set_accumulator(CVC4::Expr accumulator) {
  this->accumulator = accumulator;
  this->accumulator_set = true;
}

CVC4::Expr analysis::smt_def_builder::var(std::string name) {
  return context.var(name);
}

CVC4::Expr analysis::smt_def_builder::var_def(std::string name) {
  return context.var(name + "_def");
}

void analysis::smt_def_builder::visit_id(gdsl::rreil::id *i, size_t rev) {
  Expr result;
  if(!rev) {
    auto &man = context.get_manager();
    Expr zero = man.mkConst(BitVector(64, (unsigned long int)(0)));
    result = zero;
  } else {
    auto i_str = i->to_string();
    result = (this->*var_current)(i_str);
  }
  set_accumulator(result);
}

void analysis::smt_def_builder::visit(summy::rreil::ssa_id *si) {
  visit_id(si, si->get_version());
}

void smt_def_builder::_default(gdsl::rreil::id *i) {
  visit_id(i, 0);
}

void analysis::smt_def_builder::visit(gdsl::rreil::variable *v) {
  v->get_id()->accept(*this);
  size_t size = current_size();
  size_t offset = v->get_offset();
  if(offset > 0 || size < 64) {
    Expr c = pop_accumulator();
    auto &man = context.get_manager();
    set_accumulator(man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(offset + size - 1, offset)), c));
  }
}

CVC4::Expr analysis::smt_def_builder::defined_boolbv(CVC4::Expr a) {
  auto &man = context.get_manager();
  size_t size = current_size();

  Expr fully = man.mkConst(BitVector(size, (unsigned long int)(-1)));
  Expr defined = man.mkExpr(kind::BITVECTOR_COMP, a, fully);

  return defined;
}

CVC4::Expr analysis::smt_def_builder::defined(CVC4::Expr a) {
  auto &man = context.get_manager();
  size_t size = current_size();

  Expr defined = defined_boolbv(a);
  return man.mkExpr(kind::BITVECTOR_REPEAT, man.mkConst(BitVectorRepeat(size)), defined);
}

CVC4::Expr analysis::smt_def_builder::defined_boolbv(CVC4::Expr a, CVC4::Expr b) {
  auto &man = context.get_manager();
  Expr anded = man.mkExpr(kind::BITVECTOR_AND, a, b);
  return defined_boolbv(anded);
}

CVC4::Expr analysis::smt_def_builder::defined(CVC4::Expr a, CVC4::Expr b) {
  auto &man = context.get_manager();
  Expr anded = man.mkExpr(kind::BITVECTOR_AND, a, b);
  return defined(anded);
}

void analysis::smt_def_builder::visit(gdsl::rreil::lin_binop *a) {
//  auto &man = context.get_manager();
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
  auto &man = context.get_manager();
  Expr imm = man.mkConst(BitVector(current_size(), (unsigned long int)(-1)));
  set_accumulator(imm);
}

void analysis::smt_def_builder::visit(gdsl::rreil::lin_scale *a) {
  auto &man = context.get_manager();
  Expr factor_bv = man.mkConst(BitVector(64, (unsigned long int)a->get_const()));
  a->get_opnd()->accept(*this);
  Expr opnd = pop_accumulator();
  Expr r = man.mkExpr(kind::BITVECTOR_MULT, factor_bv, opnd);
  set_accumulator(r);
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
  auto &man = context.get_manager();
  Expr i_exp = man.mkConst(BitVector(current_size(), (unsigned long int)(0)));
  set_accumulator(i_exp);
}


void analysis::smt_def_builder::visit(gdsl::rreil::expr_binop *eb) {
  eb->get_opnd1()->accept(*this);
  Expr opnd1 = pop_accumulator();
  eb->get_opnd2()->accept(*this);
  Expr opnd2 = pop_accumulator();

  auto &man = context.get_manager();
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
      result = man.mkExpr(kind::BITVECTOR_AND, opnd1, opnd2);
      break;
    }
    case BIN_OR: {
      result = man.mkExpr(kind::BITVECTOR_AND, opnd1, opnd2);
      break;
    }
    case BIN_XOR: {
      result = man.mkExpr(kind::BITVECTOR_AND, opnd1, opnd2);
      break;
    }
    default: {
      throw string("Invalid expression");
    }
  }
  set_accumulator(result);
}

void analysis::smt_def_builder::visit(gdsl::rreil::expr_ext *ext) {
  size_t to_size = current_size();
  push_size(ext->get_fromsize());
  ext->get_opnd()->accept(*this);
  Expr opnd = pop_accumulator();
  pop_size();

  auto &man = context.get_manager();
  Expr result;
  switch(ext->get_op()) {
    case EXT_ZX: {
      Expr upper_bits = man.mkConst(BitVector(to_size - ext->get_fromsize(), (unsigned long int)(-1)));
      result = man.mkExpr(kind::BITVECTOR_CONCAT, upper_bits, opnd);
      break;
    }
    case EXT_SX: {
      result = man.mkExpr(kind::BITVECTOR_SIGN_EXTEND, man.mkConst(BitVectorSignExtend(to_size - ext->get_fromsize())),
          opnd);
      break;
    }
    default: {
      throw string("Invalid extension");
    }
  }
  set_accumulator(result);
}

void analysis::smt_def_builder::visit(gdsl::rreil::address *addr) {
  push_size(addr->get_size());
  addr->get_lin()->accept(*this);
  pop_size();
}

CVC4::Expr analysis::smt_def_builder::concat_rhs(id *lhs_id, size_t size, size_t offset, Expr rhs) {
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
    Expr id_old_exp = (this->*var_current)(id_old->to_string());
    delete id_old;

    struct slice {
      function<Expr(ExprManager&)> get;
      slice(Expr e, unsigned high, unsigned low) {
        get = [&](ExprManager &man) {
          auto extract_upper = BitVectorExtract(high, low);
          return man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(extract_upper), e);
        };
      }
      slice(Expr e) {
        get = [&](ExprManager &man) {
          return e;
        };
      }
    };

    auto concat = [&](vector<slice> slices) {
      vector<Expr> exprs;
      for(auto &slice : slices)
        exprs.push_back(slice.get(man));
      rhs_conc = man.mkExpr(kind::BITVECTOR_CONCAT, exprs);
    };

    if(offset == 0) concat( { slice(id_old_exp, 63, size), slice(rhs) });
    else if(offset + size == 64) concat( { slice(rhs), slice(id_old_exp, offset - 1, 0) });
    else concat( { slice(id_old_exp, 63, size + offset), slice(rhs), slice(id_old_exp, offset - 1, 0) });
  }
  return rhs_conc;
}

void analysis::smt_def_builder::handle_assign(size_t size, gdsl::rreil::variable *lhs_gr, std::function<void()> rhs_accept) {
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

void smt_def_builder::visit(gdsl::rreil::assign *a) {
  handle_assign(rreil_prop::size_of_assign(a), a->get_lhs(), [&]() {
    a->get_rhs()->accept(*this);
  });
}

CVC4::Expr analysis::smt_def_builder::enforce_aligned(size_t size, CVC4::Expr address) {
  auto &man = context.get_manager();
//  size_t addr_low_real_sz = log2(size/8);
//  Expr addr_low_real = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(addr_low_real_sz - 1, 0)), address);
//  Expr addr_constr = man.mkExpr(kind::EQUAL, addr_low_real, man.mkConst(BitVector(addr_low_real_sz, (unsigned long int)0)));
  Expr addr_constr = man.mkConst(true);
  return addr_constr;
}

CVC4::Expr analysis::smt_def_builder::extract_lower_bit_addr(CVC4::Expr address) {
  auto &man = context.get_manager();
  Expr lower_addr_bits = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(2, 0)), address);
  Expr lower_extended = man.mkExpr(kind::BITVECTOR_ZERO_EXTEND, man.mkConst(BitVectorZeroExtend(64 - 3 - 3)),
      lower_addr_bits);
  Expr lower_bit_addr = man.mkExpr(kind::BITVECTOR_CONCAT, lower_extended, man.mkConst(BitVector(3, (unsigned long int)0)));
  return lower_bit_addr;
}

void analysis::smt_def_builder::visit(gdsl::rreil::load *l) {
  var_current = &smt_def_builder::var_def;
  push_size(l->get_size());
//  l->get_address()->accept(*this);
//  Expr address = pop_accumulator();
  smt_builder sb(context, rd_result);
  Expr address = sb.build(l->get_address());

  var_current = &smt_def_builder::var_def;
  l->get_lhs()->get_id()->accept(*this);
  Expr lhs = pop_accumulator();
  pop_size();

  size_t mem_rev = rd_result.result[from]->get_memory_rev();
  Expr memory = context.memory_def(mem_rev);

  Expr drefed = sb.load_memory(memory, l->get_size(), address);

  Expr rhs_conc = concat_rhs(l->get_lhs()->get_id(), l->get_size(), l->get_lhs()->get_offset(), drefed);
  auto &man = context.get_manager();
  Expr load = man.mkExpr(kind::EQUAL, rhs_conc, lhs);

  Expr all = l->get_size() > 8 ? man.mkExpr(kind::AND, enforce_aligned(l->get_size(), address), load) : load;

  Expr memory_ini = context.memory_def(0);
  Expr addr_high = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(63, 3)), address);
  Expr drefed_ini = man.mkExpr(kind::SELECT, memory_ini, addr_high);
  Expr initialization = man.mkExpr(kind::EQUAL, drefed_ini, man.mkConst(BitVector(64, (unsigned long int)(0))));

  Expr all_plus_init = man.mkExpr(kind::AND, all, initialization);

//  cout << all_plus_init << endl;

  set_accumulator(all_plus_init);
}

void analysis::smt_def_builder::visit(gdsl::rreil::store *s) {
  var_current = &smt_def_builder::var_def;
  push_size(s->get_size());
  smt_builder sb(context, rd_result);
  Expr address = sb.build(s->get_address());

  var_current = &smt_def_builder::var_def;
  s->get_rhs()->accept(*this);
  Expr rhs = pop_accumulator();
  size_t size = pop_size();

  Expr memory_before = context.memory_def(rd_result.result[from]->get_memory_rev());

  auto &man = context.get_manager();
  Expr mem_stored = sb.store_memory(memory_before, size, address, rhs);

  Expr memory_after = context.memory_def(rd_result.result[to]->get_memory_rev());
  Expr store = man.mkExpr(kind::EQUAL, memory_after, mem_stored);

  Expr all = size > 8 ? man.mkExpr(kind::AND, enforce_aligned(size, address), store) : store;
  set_accumulator(all);
}

CVC4::Expr analysis::smt_def_builder::build(gdsl::rreil::statement *s) {
  var_current = &smt_def_builder::var_def;

  s->accept(*this);
  return pop_accumulator();
}

CVC4::Expr analysis::smt_def_builder::build(cfg::phi_assign const *pa) {
  var_current = &smt_def_builder::var_def;

  handle_assign(pa->get_size(), pa->get_lhs(), [&]() {
    pa->get_rhs()->accept(*this);
  });
  return pop_accumulator();
}

CVC4::Expr analysis::smt_def_builder::build_target(gdsl::rreil::address *addr) {
  var_current = &smt_def_builder::var_def;

  addr->accept(*this);
  push_size(addr->get_size());
  auto &man = context.get_manager();
  Expr result = man.mkExpr(kind::EQUAL, defined_boolbv(pop_accumulator()), man.mkConst(BitVector(1, (unsigned long int)(1))));
  pop_size();
  return result;
}

void analysis::smt_def_builder::edge(size_t from, size_t to) {
  this->from = from;
  this->to = to;
}
