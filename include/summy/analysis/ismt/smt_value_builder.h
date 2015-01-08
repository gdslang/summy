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
#include <summy/analysis/ismt/smt_builder.h>
#include <summy/cfg/edge/phi_edge.h>
#include <cvc4/cvc4.h>
#include <vector>
#include <memory>

namespace analysis {

class smt_value_builder: public smt_builder {
private:
  CVC4::Expr get_id_old_exp(gdsl::rreil::id *id, size_t def_node);

  void _default(gdsl::rreil::id *i);

  void visit(gdsl::rreil::lin_binop *a);
  void visit(gdsl::rreil::lin_imm *a);
  void visit(gdsl::rreil::lin_scale *a);

  void visit(gdsl::rreil::expr_cmp *ec);
  void visit(gdsl::rreil::arbitrary *ab);

  void visit(gdsl::rreil::expr_binop *eb);
  void visit(gdsl::rreil::expr_ext *ext);

  CVC4::Expr enforce_aligned(size_t size, CVC4::Expr address);

  void visit(gdsl::rreil::load *l);
  void visit(gdsl::rreil::store *s);
public:
  smt_value_builder(cvc_context &context, adaptive_rd::adaptive_rd_result rd_result) :
    smt_builder(context, rd_result) {
  }

  CVC4::Expr load_memory(CVC4::Expr memory, size_t size, CVC4::Expr address);
  CVC4::Expr store_memory(CVC4::Expr memory_before, size_t size, CVC4::Expr address, CVC4::Expr value);

  using smt_builder::build;
  CVC4::Expr build(gdsl::rreil::address *addr);
};

}  // namespace analysis

