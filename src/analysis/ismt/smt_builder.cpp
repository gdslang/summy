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

size_t analysis::smt_builder::current_size() {
  if(sizes.empty())
    throw string("Size unknown");
  return sizes.back();
}

size_t analysis::smt_builder::pop_size() {
  if(sizes.empty())
    throw string("Empty size stack");
  size_t back = sizes.back();
  sizes.pop_back();
  return back;
}

void analysis::smt_builder::push_size(size_t size) {
  sizes.push_back(size);
}

void analysis::smt_builder::replace_size(size_t size) {
  pop_size();
  push_size(size);
}

CVC4::Expr analysis::smt_builder::current_accumulator() {
  if(!accumulator_set) throw string(":/");
  return accumulator;
}

CVC4::Expr analysis::smt_builder::pop_accumulator() {
  if(!accumulator_set) throw string(":/");
  this->accumulator_set = false;
  return accumulator;
}

void analysis::smt_builder::set_accumulator(CVC4::Expr accumulator) {
  this->accumulator = accumulator;
  this->accumulator_set = true;
}



CVC4::Expr analysis::smt_builder::concat_rhs(id const *lhs_id, size_t size, size_t offset, Expr rhs) {
  auto &man = context.get_manager();
  Expr rhs_conc;
  if(size == 0 || size == 64) rhs_conc = rhs;
  else {
    shared_ptr<id> lhs_id_wrapped(lhs_id->copy());

    auto def_it = rd_result.result[from]->get_elements().find(lhs_id_wrapped);
    size_t def_node = 0;
    if(def_it != rd_result.result[from]->get_elements().end()) def_node = def_it->second;
    sr::copy_visitor civ;
    civ._([&](std::unique_ptr<gdsl::rreil::id> inner, int_t rev) {
      return inner;
    });
    lhs_id_wrapped->accept(civ);
    id *id_old = def_node > 0 ? new sr::ssa_id(civ.retrieve_id().release(), def_node) : civ.retrieve_id().release();
    Expr id_old_exp = get_id_old_exp(id_old, def_node);
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

void analysis::smt_builder::handle_assign(size_t size, gdsl::rreil::variable const *lhs_gr, std::function<void()> rhs_accept) {
  /*
   * Todo: what if size == 0?
   */
//    base::visit(a);
  auto &man = context.get_manager();
  sizes.push_back(size);
  rhs_accept();
  Expr rhs = pop_accumulator();
  lhs_gr->get_id().accept(*this);
  Expr lhs = pop_accumulator();
  size = pop_size();

//  int_t ass_size = rreil_prop::size_of_assign(a);
  int_t ass_size = size;
  int_t offset = lhs_gr->get_offset();

  Expr rhs_conc = concat_rhs(&lhs_gr->get_id(), ass_size, offset, rhs);

  Expr ass = man.mkExpr(kind::EQUAL, rhs_conc, lhs);
  set_accumulator(ass);

//    cout << ass << endl;
////    context.get_smtEngine().checkSat(man.mkExpr(kind::EQUAL, result, result));
//    context.get_smtEngine().checkSat(ass);
//    cout << ":-)" << endl;
}

void analysis::smt_builder::visit(gdsl::rreil::variable const *v) {
  v->get_id().accept(*this);
  size_t size = current_size();
  size_t offset = v->get_offset();
  if(offset > 0 || size < 64) {
    Expr c = pop_accumulator();
    auto &man = context.get_manager();
    set_accumulator(man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(offset + size - 1, offset)), c));
  }
}

void analysis::smt_builder::visit(gdsl::rreil::address const *addr) {
  push_size(addr->get_size());
  addr->get_lin().accept(*this);
  pop_size();
}

void smt_builder::visit(gdsl::rreil::assign const *a) {
  handle_assign(a->get_size(), &a->get_lhs(), [&]() {
    a->get_rhs().accept(*this);
  });
}

CVC4::Expr analysis::smt_builder::extract_lower_bit_addr(CVC4::Expr address) {
  auto &man = context.get_manager();
  Expr lower_addr_bits = man.mkExpr(kind::BITVECTOR_EXTRACT, man.mkConst(BitVectorExtract(2, 0)), address);
  Expr lower_extended = man.mkExpr(kind::BITVECTOR_ZERO_EXTEND, man.mkConst(BitVectorZeroExtend(64 - 3 - 3)),
      lower_addr_bits);
  Expr lower_bit_addr = man.mkExpr(kind::BITVECTOR_CONCAT, lower_extended, man.mkConst(BitVector(3, (unsigned long int)0)));
  return lower_bit_addr;
}

CVC4::Expr analysis::smt_builder::build(gdsl::rreil::statement const *s) {
  s->accept(*this);
  return pop_accumulator();
}

CVC4::Expr analysis::smt_builder::build(cfg::phi_assign const *pa) {
  handle_assign(pa->get_size(), &pa->get_lhs(), [&]() {
    pa->get_rhs().accept(*this);
  });
  return pop_accumulator();
}

CVC4::Expr analysis::smt_builder::build(cfg::phi_memory const& pm) {
  auto &man = context.get_manager();
  return man.mkExpr(kind::EQUAL, context.memory(pm.from), context.memory(pm.to));
}

void analysis::smt_builder::edge(size_t from, size_t to) {
  this->from = from;
  this->to = to;
}
