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
  typedef std::function<std::tuple<bool, int_t>(gdsl::rreil::variable const*)> variable_callback_t;
private:
  variable_callback_t variable_callback;
public:
  rreil_evaluator() : variable_callback(NULL) {
  }
  rreil_evaluator(variable_callback_t variable_callback) : variable_callback(variable_callback) {
  }

  std::tuple<bool, int_t> evaluate(gdsl::rreil::expr const *expr);
  std::tuple<bool, int_t> evaluate(gdsl::rreil::linear const *lin);
};

struct rreil_prop {
  static bool is_ip(gdsl::rreil::variable const *v);
  static int_t size_of_rhs(gdsl::rreil::assign const *a);
};
