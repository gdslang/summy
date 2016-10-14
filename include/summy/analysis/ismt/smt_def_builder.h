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
#include <summy/analysis/ismt/smt_builder.h>
#include <summy/cfg/edge/phi_edge.h>
#include <cvc4/cvc4.h>
#include <vector>
#include <memory>

/*
 * Todo: Gemeinsame Superklasse mit smt_builder
 */

namespace analysis {

class smt_def_builder: public smt_builder {
private:
  CVC4::Expr get_id_old_exp(gdsl::rreil::id const *id, size_t def_node) override;

  CVC4::Expr defined_boolbv(CVC4::Expr a);
  CVC4::Expr defined(CVC4::Expr a);
  CVC4::Expr defined_boolbv(CVC4::Expr a, CVC4::Expr b);
  CVC4::Expr defined(CVC4::Expr a, CVC4::Expr b);

  CVC4::Expr var(std::string name);
  CVC4::Expr var_def(std::string name);
  CVC4::Expr id_at_rev(gdsl::rreil::id const *i, size_t rev);

  void _default(gdsl::rreil::id const *i) override;
  void visit(summy::rreil::ssa_id const *si) override;

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

  using smt_builder::build;
  CVC4::Expr build_target(gdsl::rreil::address const *addr);
};

}  // namespace analysis
