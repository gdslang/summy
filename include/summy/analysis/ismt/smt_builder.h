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

class smt_builder: protected summy::rreil::visitor {
protected:
//  using base = summy::rreil::visitor;
  std::vector<size_t> sizes;
  bool accumulator_set = false;
  CVC4::Expr accumulator;

  cvc_context &context;
  CVC4::ExprManager &manager;

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

  virtual CVC4::Expr get_id_old_exp(gdsl::rreil::id *id, size_t def_node) = 0;
  CVC4::Expr concat_rhs(gdsl::rreil::id *lhs_id, size_t size, size_t offset, CVC4::Expr rhs);
  void handle_assign(size_t size, gdsl::rreil::variable *lhs_, std::function<void()> rhs_accept);

  void visit(gdsl::rreil::variable *v);
  void visit(gdsl::rreil::address *addr);
  void visit(gdsl::rreil::assign *a);

  CVC4::Expr extract_lower_bit_addr(CVC4::Expr address);
public:
  smt_builder(cvc_context &context, adaptive_rd::adaptive_rd_result rd_result) :
      context(context), manager(context.get_manager()), rd_result(rd_result) {
  }

  CVC4::Expr build(gdsl::rreil::statement *s);
  CVC4::Expr build(cfg::phi_assign const *pa);
  CVC4::Expr build(cfg::phi_memory const& pm);
  void edge(size_t from, size_t to);
};

}  // namespace analysis

