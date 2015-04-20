/*
 * converter.h
 *
 *  Created on: Mar 16, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <stdint.h>
#include <cppgdsl/rreil/linear/linear.h>
#include <cppgdsl/rreil/sexpr/sexpr.h>
#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/value_set/value_set.h>
#include <cppgdsl/rreil/expr/expr.h>
#include <functional>
#include <memory>
#include <vector>

namespace analysis {
namespace api {

typedef  std::function<num_linear*(std::shared_ptr<gdsl::rreil::id>, size_t, size_t)> transLE_t;

struct expr_cmp_result_t {
  analysis::api::num_expr_cmp *primary;
  std::vector<analysis::api::num_expr_cmp*> additional;

  /*
   * dtor...
   */
};

class converter {
private:
  size_t size;
  transLE_t transLE;

  num_var *conv_id(gdsl::rreil::id *id);

  num_linear *conv_linear(gdsl::rreil::linear *lin, int64_t scale);
  expr_cmp_result_t conv_expr_cmp(gdsl::rreil::sexpr_cmp *se);
public:
  converter(size_t size, transLE_t transLE) : size(size), transLE(transLE) {
  }

  expr_cmp_result_t conv_expr_cmp(gdsl::rreil::sexpr *se);
  analysis::api::num_expr *conv_expr(gdsl::rreil::sexpr *se);
  analysis::api::num_expr *conv_expr(gdsl::rreil::expr *expr);
  analysis::api::num_expr *conv_expr(gdsl::rreil::linear *lin);
  num_linear *conv_linear(gdsl::rreil::linear *lin);

  static num_linear *mul(int64_t scale, num_linear *a);
  static num_linear *add(num_linear *a, summy::vs_shared_t vs);
  static num_linear *add(num_linear *a, num_linear *b);
};



}
}
