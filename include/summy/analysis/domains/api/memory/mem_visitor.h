/*
 * mem_visitor.h
 *
 *  Created on: Mar 25, 2015
 *      Author: Julian Kranz
 */
#pragma once
#include <functional>

namespace analysis {
namespace api {

class mem_expr_re;
class mem_expr_deref;

class mem_visitor {
public:
  typedef std::function<void(mem_expr_re*)> mem_expr_re_callback_t;
  typedef std::function<void(mem_expr_deref*)> mem_expr_deref_callback_t;
  typedef std::function<void()> default_callback_t;
private:
  bool ignore_default;

  mem_expr_re_callback_t mem_expr_re_callback = NULL;
  mem_expr_deref_callback_t mem_expr_deref_callback = NULL;
  default_callback_t default_callback = NULL;
public:
  virtual ~mem_visitor() {
  }
  mem_visitor(bool ignore_default = false) :
      ignore_default(ignore_default) {
  }

  virtual void visit(mem_expr_re *v);
  virtual void visit(mem_expr_deref *v);
  virtual void _default();

  void _(mem_expr_re_callback_t c) {
    this->mem_expr_re_callback = c;
  }

  void _(mem_expr_deref_callback_t c) {
    this->mem_expr_deref_callback = c;
  }

  void _default(default_callback_t c) {
    this->default_callback = c;
  }
};


}
}
