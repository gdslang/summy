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
  std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs_before;
  std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs_after;
  bool rhs = false;

  cvc_context &context;
  std::vector<CVC4::Expr> sub_exprs;

  CVC4::Expr pop();

  CVC4::Expr concat_rhs(gdsl::rreil::id *lhs_id, size_t size, size_t offset, CVC4::Expr rhs);
  void handle_assign(size_t size, gdsl::rreil::variable *lhs_, std::function<void()> rhs_accept);

  void _default(gdsl::rreil::id *i);
  void visit(gdsl::rreil::variable *v);

  void visit(gdsl::rreil::lin_binop *a);
  void visit(gdsl::rreil::lin_imm *a);
  void visit(gdsl::rreil::lin_scale *a);

  void visit(gdsl::rreil::assign *a);

  void visit(gdsl::rreil::load *l);
  void visit(gdsl::rreil::store *s);
public:
  smt_builder(cvc_context &context) :
      context(context) {
  }
  CVC4::Expr build(gdsl::rreil::statement *s, std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs_befores,
      std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs_after);
  CVC4::Expr build(cfg::phi_assign const *pa, std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs_before,
      std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs_after);
};

}  // namespace analysis

