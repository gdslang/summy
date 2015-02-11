/*
 * sexpr_visitor.h
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/sexpr/sexpr_visitor.h>
#include <functional>

namespace summy {
namespace rreil {

class value_set;

class id_visitor : public virtual gdsl::rreil::sexpr_visitor {
public:
  typedef std::function<void(value_set*)> value_set_callback_t;
private:
  value_set_callback_t value_set_callback = NULL;
public:
  virtual ~id_visitor() {
  }

  virtual void visit(value_set *a);

  void _(value_set_callback_t c) {
    this->value_set_callback = c;
  }
};

}
}
