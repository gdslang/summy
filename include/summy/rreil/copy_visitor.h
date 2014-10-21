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
private:
  std::function<ssa_id*(gdsl::rreil::id*, int_t)> ssa_id_ctor = NULL;

public:
  virtual void visit(ssa_id *a);

  using summy::rreil::id_visitor::_;
  using gdsl::rreil::copy_visitor::_;
};


}  // namespace rreil
}  // namespace summy
