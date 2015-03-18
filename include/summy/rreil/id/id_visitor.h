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
class numeric_id;
class memory_id;

class id_visitor : public virtual gdsl::rreil::id_visitor {
private:
  std::function<void(ssa_id*)> ssa_id_callback = NULL;
  std::function<void(numeric_id*)> numeric_id_callback = NULL;
  std::function<void(memory_id*)> memory_id_callback = NULL;
public:
  virtual ~id_visitor() {
  }

  virtual void visit(ssa_id *a);
  virtual void visit(numeric_id *a);
  virtual void visit(memory_id *a);

  void _(std::function<void(ssa_id*)> c) {
    this->ssa_id_callback = c;
  }
  void _(std::function<void(numeric_id*)> c) {
    this->numeric_id_callback = c;
  }
  void _(std::function<void(memory_id*)> c) {
    this->memory_id_callback = c;
  }
};

}
}
