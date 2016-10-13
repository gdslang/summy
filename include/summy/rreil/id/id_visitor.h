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
class ptr_memory_id;
class allocation_memory_id;
class sm_id;
class special_ptr;

class id_visitor : public virtual gdsl::rreil::id_visitor {
private:
  std::function<void(ssa_id const *)> ssa_id_callback = NULL;
  std::function<void(numeric_id const *)> numeric_id_callback = NULL;
  std::function<void(ptr_memory_id const *)> ptr_memory_id_callback = NULL;
  std::function<void(allocation_memory_id const *)> allocation_memory_id_callback = NULL;
  std::function<void(sm_id const *)> sm_id_callback = NULL;
  std::function<void(special_ptr const *)> spcial_ptr_callback = NULL;

public:
  virtual ~id_visitor() {}

  virtual void visit(ssa_id const *a);
  virtual void visit(numeric_id const *a);
  virtual void visit(ptr_memory_id const *a);
  virtual void visit(allocation_memory_id const *a);
  virtual void visit(sm_id const *a);
  virtual void visit(special_ptr const *a);

  void _(std::function<void(ssa_id const *)> c) {
    this->ssa_id_callback = c;
  }
  void _(std::function<void(numeric_id const *)> c) {
    this->numeric_id_callback = c;
  }
  void _(std::function<void(ptr_memory_id const *)> c) {
    this->ptr_memory_id_callback = c;
  }
  void _(std::function<void(allocation_memory_id const *)> c) {
    this->allocation_memory_id_callback = c;
  }
  void _(std::function<void(sm_id const *)> c) {
    this->sm_id_callback = c;
  }
  void _(std::function<void(special_ptr const *)> c) {
    this->spcial_ptr_callback = c;
  }

  using gdsl::rreil::id_visitor::_;
};
}
}
