/*
 * smt_builder.h
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/rreil/visitor.h>
#include <summy/analysis/ismt/cvc_context.h>
#include <cvc4/cvc4.h>
#include <vector>

namespace analysis {

class smt_builder : public summy::rreil::visitor {
private:
//  using base = summy::rreil::visitor;

  cvc_context &context;
  std::vector<CVC4::Expr> sub_exprs;

  CVC4::Expr pop();
public:
  void _default(gdsl::rreil::id *i);
//  void visit(gdsl::rreil::variable *v);
  void visit(gdsl::rreil::assign *a);

  void visit(gdsl::rreil::lin_binop *a);
  void visit(gdsl::rreil::lin_imm *a);
  void visit(gdsl::rreil::lin_scale *a);

  smt_builder(cvc_context &context) : context(context) {
  }
  CVC4::Expr build(gdsl::rreil::statement *s);
};

}  // namespace analysis


