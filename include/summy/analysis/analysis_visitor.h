/*
 * num_visitor.h
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <functional>

namespace analysis {

class fp_analysis;
class summary_dstack;

class analysis_visitor {
public:
  typedef std::function<void(summary_dstack*)> summary_dstack_callback_t;
  typedef std::function<void()> default_callback_t;
private:
  bool ignore_default;

  summary_dstack_callback_t summary_dstack_callback = NULL;
  default_callback_t default_callback = NULL;
public:
  virtual ~analysis_visitor() {
  }
  analysis_visitor(bool ignore_default = false) :
      ignore_default(ignore_default) {
  }

  virtual void visit(summary_dstack *v);
  virtual void visit(fp_analysis *v);
  virtual void _default();

  void _(summary_dstack_callback_t c) {
    this->summary_dstack_callback = c;
  }

  void _default(default_callback_t c) {
    this->default_callback = c;
  }
};


}
