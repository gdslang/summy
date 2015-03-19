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

namespace analysis {
namespace api {

typedef  std::function<num_linear*(std::shared_ptr<gdsl::rreil::id>, size_t)> transLE_t;

class converter {
private:
  transLE_t transLE;

  num_var *conv_id(gdsl::rreil::id *id);
  num_linear *add(num_linear *a, summy::vs_shared_t vs);
  num_linear *add(num_linear *a, num_linear *b);
  num_linear *conv_linear(gdsl::rreil::linear *lin, int64_t scale);
  num_linear *conv_linear(gdsl::rreil::linear *lin);
  num_expr *conv_sexpr(gdsl::rreil::sexpr *se);
public:
  converter(transLE_t transLE) : transLE(transLE) {
  }

  analysis::api::num_expr *conv_expr(gdsl::rreil::expr *expr);
};



}
}
