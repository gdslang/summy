/*
 * copy_visitor.h
 *
 *  Created on: Oct 20, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/copy_visitor.h>
#include <summy/rreil/visitor.h>

namespace summy {
namespace rreil {

class ssa_id;

class copy_visitor : public gdsl::rreil::copy_visitor, public virtual summy::rreil::id_visitor {
public:
  typedef std::function<gdsl::rreil::id*(gdsl::rreil::id*, int_t)> ssa_id_ctor_t;
private:
  ssa_id_ctor_t ssa_id_ctor = NULL;

public:
  virtual void visit(ssa_id *a);

  using summy::rreil::id_visitor::_;
  using gdsl::rreil::copy_visitor::_;

  void _(ssa_id_ctor_t c) {
    ssa_id_ctor = c;
  }
};


}  // namespace rreil
}  // namespace summy
