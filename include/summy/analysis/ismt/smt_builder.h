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
#include <cvc4/cvc4.h>
#include <vector>
#include <memory>

namespace analysis {

class smt_builder: public summy::rreil::visitor {
private:
//  using base = summy::rreil::visitor;
  std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs;

  cvc_context &context;
  std::vector<CVC4::Expr> sub_exprs;

  CVC4::Expr pop();
  CVC4::Expr id_by_string(std::string s);
public:
  void _default(gdsl::rreil::id *i);
//  void visit(gdsl::rreil::variable *v);
  void visit(gdsl::rreil::assign *a);

  void visit(gdsl::rreil::lin_binop *a);
  void visit(gdsl::rreil::lin_imm *a);
  void visit(gdsl::rreil::lin_scale *a);

  smt_builder(cvc_context &context) :
      context(context) {
  }
  CVC4::Expr build(gdsl::rreil::statement *s, std::shared_ptr<analysis::adaptive_rd::adaptive_rd_elem> defs);
};

}  // namespace analysis

