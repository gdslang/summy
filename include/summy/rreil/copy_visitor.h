/*
 * copy_visitor.h
 *
 *  Created on: Oct 20, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/copy_visitor.h>
#include <summy/rreil/visitor.h>
#include <memory>
#include <string>
#include <experimental/optional>

namespace summy {
namespace rreil {

class ssa_id;

class copy_visitor : public gdsl::rreil::copy_visitor, public virtual summy::rreil::id_visitor {
public:
  typedef std::function<gdsl::rreil::id *(gdsl::rreil::id *, int_t)> ssa_id_ctor_t;
  typedef std::function<gdsl::rreil::id *(
    int_t, std::experimental::optional<std::string>, std::experimental::optional<bool>)> numeric_id_ctor_t;
  typedef std::function<gdsl::rreil::id *(std::shared_ptr<gdsl::rreil::id>)> ptr_memory_id_ctor_t;
  typedef std::function<gdsl::rreil::id *(size_t)> allocation_memory_id_ctor_t;
  typedef std::function<gdsl::rreil::id *(std::string, void *)> sm_id_ctor_t;

private:
  ssa_id_ctor_t ssa_id_ctor = NULL;
  numeric_id_ctor_t numeric_id_ctor = NULL;
  ptr_memory_id_ctor_t ptr_memory_id_ctor = NULL;
  allocation_memory_id_ctor_t allocation_memory_id_ctor = NULL;
  sm_id_ctor_t sm_id_ctor = NULL;

public:
  virtual void visit(ssa_id const *a);
  virtual void visit(numeric_id const *a);
  virtual void visit(ptr_memory_id const *a);
  virtual void visit(allocation_memory_id const *a);
  virtual void visit(sm_id *a);

  using summy::rreil::id_visitor::_;
  using gdsl::rreil::copy_visitor::_;

  void _(ssa_id_ctor_t c) {
    ssa_id_ctor = c;
  }
  void _(numeric_id_ctor_t c) {
    numeric_id_ctor = c;
  }
  void _(ptr_memory_id_ctor_t c) {
    ptr_memory_id_ctor = c;
  }
  void _(allocation_memory_id_ctor_t c) {
    allocation_memory_id_ctor = c;
  }
  void _(sm_id_ctor_t c) {
    sm_id_ctor = c;
  }
};

} // namespace rreil
} // namespace summy
