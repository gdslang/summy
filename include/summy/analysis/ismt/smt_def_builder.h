/*
 * smt_def_builder.h
 *
 *  Created on: Dec 15, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/rreil/id/ssa_id.h>
#include <summy/rreil/visitor.h>
#include <summy/analysis/ismt/cvc_context.h>
#include <summy/analysis/adaptive_rd/adaptive_rd.h>
#include <summy/cfg/edge/phi_edge.h>
#include <cvc4/cvc4.h>
#include <vector>
#include <memory>

/*
 * Todo: Gemeinsame Superklasse mit smt_builder
 */

namespace analysis {

class smt_def_builder: private summy::rreil::visitor {
private:
//  using base = summy::rreil::visitor;
  std::vector<size_t> sizes;
  bool accumulator_set = false;
  CVC4::Expr accumulator;

  cvc_context &context;
  adaptive_rd::adaptive_rd_result rd_result;
  size_t from = 0;
  size_t to = 0;

  size_t current_size();
  size_t pop_size();
  void push_size(size_t size);
  void replace_size(size_t size);

  CVC4::Expr current_accumulator();
  CVC4::Expr pop_accumulator();
  void set_accumulator(CVC4::Expr accumulator);

  CVC4::Expr concat_rhs(gdsl::rreil::id *lhs_id, size_t size, size_t offset, CVC4::Expr rhs);
  void handle_assign(size_t size, gdsl::rreil::variable *lhs_, std::function<void()> rhs_accept);

  CVC4::Expr defined_boolbv(CVC4::Expr a);
  CVC4::Expr defined(CVC4::Expr a);
  CVC4::Expr defined_boolbv(CVC4::Expr a, CVC4::Expr b);
  CVC4::Expr defined(CVC4::Expr a, CVC4::Expr b);

  CVC4::Expr var(std::string name);
  void visit_id(gdsl::rreil::id *i, size_t rev);
  void _default(gdsl::rreil::id *i);
  void visit(summy::rreil::ssa_id *si);

  void visit(gdsl::rreil::variable *v);

  void visit(gdsl::rreil::lin_binop *a);
  void visit(gdsl::rreil::lin_imm *a);
  void visit(gdsl::rreil::lin_scale *a);

  void visit(gdsl::rreil::expr_cmp *ec);
  void visit(gdsl::rreil::arbitrary *ab);

  void visit(gdsl::rreil::expr_binop *eb);
  void visit(gdsl::rreil::expr_ext *ext);

  void visit(gdsl::rreil::address *addr);

  void visit(gdsl::rreil::assign *a);

  CVC4::Expr enforce_aligned(size_t size, CVC4::Expr address);
  CVC4::Expr extract_lower_bit_addr(CVC4::Expr address);
  void visit(gdsl::rreil::load *l);
  void visit(gdsl::rreil::store *s);
public:
  smt_def_builder(cvc_context &context, adaptive_rd::adaptive_rd_result rd_result) :
      context(context), rd_result(rd_result) {
  }
  CVC4::Expr build(gdsl::rreil::statement *s);
  CVC4::Expr build(cfg::phi_assign const *pa);
  CVC4::Expr build_target(gdsl::rreil::address *addr);
  void edge(size_t from, size_t to);
};

}  // namespace analysis
