/*
 * value_set_visitor.h
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <functional>

namespace summy {

class value_set;
class vs_finite;
class vs_open;
class vs_top;

class value_set_visitor {
public:
  typedef std::function<void(vs_finite*)> vs_finite_callback_t;
  typedef std::function<void(vs_open*)> vs_open_callback_t;
  typedef std::function<void(vs_top*)> vs_top_callback_t;
  typedef std::function<void(value_set*)> default_callback_t;
private:
  bool ignore_default;

  vs_finite_callback_t vs_finite_callback = NULL;
  vs_open_callback_t vs_open_callback = NULL;
  vs_top_callback_t vs_top_callback = NULL;
  default_callback_t default_callback = NULL;
public:
  virtual ~value_set_visitor() {
  }
  value_set_visitor(bool ignore_default = false) :
      ignore_default(ignore_default) {
  }

  virtual void visit(vs_finite *v);
  virtual void visit(vs_open *v);
  virtual void visit(vs_top *v);
  virtual void _default(value_set *v);

  void _(vs_finite_callback_t c) {
    this->vs_finite_callback = c;
  }

  void _(vs_open_callback_t c) {
    this->vs_open_callback = c;
  }

  void _(vs_top_callback_t c) {
    this->vs_top_callback = c;
  }

  void _default(default_callback_t c) {
    this->default_callback = c;
  }
};

}
