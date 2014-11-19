/*
 * smt_builder.h
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/rreil/visitor.h>
#include <summy/analysis/ismt/cvc_context.h>
#include <summy/analysis/adaptive_rd/adaptive_rd.h>
#include <summy/cfg/edge/phi_edge.h>
#include <cvc4/cvc4.h>
#include <vector>
#include <memory>

namespace analysis {

class smt_builder: private summy::rreil::visitor {
private:
//  using base = summy::rreil::visitor;
  std::vector<size_t> sizes;

  cvc_context &context;
  adaptive_rd::adaptive_rd_result rd_result;
  size_t from = 0;
  size_t to = 0;

  std::vector<CVC4::Expr> sub_exprs;

  size_t current_size();
  size_t pop_size();
  void push_size(size_t size);
  void replace_size(size_t size);
  CVC4::Expr pop();

  CVC4::Expr concat_rhs(gdsl::rreil::id *lhs_id, size_t size, size_t offset, CVC4::Expr rhs);
  void handle_assign(size_t size, gdsl::rreil::variable *lhs_, std::function<void()> rhs_accept);

  void _default(gdsl::rreil::id *i);
  void visit(gdsl::rreil::variable *v);

  void visit(gdsl::rreil::lin_binop *a);
  void visit(gdsl::rreil::lin_imm *a);
  void visit(gdsl::rreil::lin_scale *a);

  void visit(gdsl::rreil::expr_cmp *ec);

  void visit(gdsl::rreil::address *addr);

  void visit(gdsl::rreil::assign *a);

  CVC4::Expr enforce_aligned(size_t size, CVC4::Expr address);
  CVC4::Expr extract_lower_bit_addr(CVC4::Expr address);
  void visit(gdsl::rreil::load *l);
  void visit(gdsl::rreil::store *s);
public:
  smt_builder(cvc_context &context, adaptive_rd::adaptive_rd_result rd_result) :
      context(context), rd_result(rd_result) {
  }
  CVC4::Expr build(gdsl::rreil::statement *s);
  CVC4::Expr build(cfg::phi_assign const *pa);
  void edge(size_t from, size_t to);
};

}  // namespace analysis

