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
  CVC4::Expr get_id_old_exp(gdsl::rreil::id const *id, size_t def_node) override;
  CVC4::Expr enforce_aligned(size_t size, CVC4::Expr address);

  void _default(gdsl::rreil::id const *i) override;

  void visit(gdsl::rreil::lin_binop const *a) override;
  void visit(gdsl::rreil::lin_imm const *a) override;
  void visit(gdsl::rreil::lin_scale const *a) override;

  void visit(gdsl::rreil::expr_cmp const *ec) override;
  void visit(gdsl::rreil::arbitrary const *ab) override;

  void visit(gdsl::rreil::expr_binop const *eb) override;
  void visit(gdsl::rreil::expr_ext const *ext) override;

  void visit(gdsl::rreil::load const *l) override;
  void visit(gdsl::rreil::store const *s) override;
public:
  using smt_builder::smt_builder;

  CVC4::Expr load_memory(CVC4::Expr memory, size_t size, CVC4::Expr address);
  CVC4::Expr store_memory(CVC4::Expr memory_before, size_t size, CVC4::Expr address, CVC4::Expr value);

  using smt_builder::build;
  CVC4::Expr build(gdsl::rreil::address const *addr);
};

}  // namespace analysis

