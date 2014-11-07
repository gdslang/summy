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

class smt_builder : private summy::rreil::visitor {
private:
  cvc_context &context;
  std::vector<CVC4::Expr> sub_exprs;

  void visit(gdsl::rreil::id *i);
//  void visit(gdsl::rreil::variable *v);
  void visit(gdsl::rreil::assign *a);
public:
  smt_builder(cvc_context &context) : context(context) {
  }
};

}  // namespace analysis


