/*
 * num_visitor.h
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <functional>

namespace analysis {
namespace api {

class num_linear_term;
class num_linear_vs;
class num_expr_cmp;
class num_expr_lin;
class num_expr_bin;

class num_visitor {
public:
  typedef std::function<void(num_linear_term*)> num_linear_term_callback_t;
  typedef std::function<void(num_linear_vs*)> num_linear_vs_callback_t;
  typedef std::function<void(num_expr_cmp*)> num_expr_cmp_callback_t;
  typedef std::function<void(num_expr_lin*)> num_expr_lin_callback_t;
  typedef std::function<void(num_expr_bin*)> num_expr_bin_callback_t;
  typedef std::function<void()> default_callback_t;
private:
  bool ignore_default;

  num_linear_term_callback_t num_linear_term_callback = NULL;
  num_linear_vs_callback_t num_linear_vs_callback = NULL;
  num_expr_cmp_callback_t num_expr_cmp_callback = NULL;
  num_expr_lin_callback_t num_expr_lin_callback = NULL;
  num_expr_bin_callback_t num_expr_bin_callback = NULL;
  default_callback_t default_callback = NULL;
public:
  virtual ~num_visitor() {
  }
  num_visitor(bool ignore_default = false) :
      ignore_default(ignore_default) {
  }

  virtual void visit(num_linear_term *v);
  virtual void visit(num_linear_vs *v);
  virtual void visit(num_expr_cmp *v);
  virtual void visit(num_expr_lin *v);
  virtual void visit(num_expr_bin *v);
  virtual void _default();

  void _(num_linear_term_callback_t c) {
    this->num_linear_term_callback = c;
  }

  void _(num_linear_vs_callback_t c) {
    this->num_linear_vs_callback = c;
  }

  void _(num_expr_cmp_callback_t c) {
    this->num_expr_cmp_callback = c;
  }

  void _(num_expr_lin_callback_t c) {
    this->num_expr_lin_callback = c;
  }

  void _(num_expr_bin_callback_t c) {
    this->num_expr_bin_callback = c;
  }

  void _default(default_callback_t c) {
    this->default_callback = c;
  }
};


}
}
