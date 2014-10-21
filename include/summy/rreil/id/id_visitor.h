/*
 * id_visitor.h
 *
 *  Created on: Oct 20, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id_visitor.h>
#include <functional>

namespace summy {
namespace rreil {

class ssa_id;

class id_visitor : public virtual gdsl::rreil::id_visitor {
private:
  std::function<void(ssa_id*)> ssa_id_callback = NULL;
public:
  virtual ~id_visitor() {
  }

  virtual void visit(ssa_id *a) {
    if(ssa_id_callback != NULL)
      ssa_id_callback(a);
    _default();
  }

  void _(std::function<void(ssa_id*)> c) {
    this->ssa_id_callback = c;
  }
};

}
}
