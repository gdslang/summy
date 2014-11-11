/*
 * rreil_evaluator.h
 *
 *  Created on: Sep 22, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/variable.h>
#include <cppgdsl/rreil/expr/expr.h>
#include <cppgdsl/rreil/linear/linear.h>
#include <cppgdsl/rreil/statement/assign.h>

#include <functional>
#include <tuple>

extern "C" {
#include <gdsl_generic.h>
}

class rreil_evaluator {
public:
  typedef std::function<std::tuple<bool, int_t>(gdsl::rreil::variable*)> variable_callback_t;
private:
  variable_callback_t variable_callback;
public:
  rreil_evaluator() : variable_callback(NULL) {
  }
  rreil_evaluator(variable_callback_t variable_callback) : variable_callback(variable_callback) {
  }

  std::tuple<bool, int_t> evaluate(gdsl::rreil::expr *expr);
  std::tuple<bool, int_t> evaluate(gdsl::rreil::linear *lin);
};

struct rreil_prop {
  static bool is_ip(gdsl::rreil::variable *v);
  static int_t size_of_assign(gdsl::rreil::assign *a);
  static int_t size_of_rhs(gdsl::rreil::assign *a);
};
