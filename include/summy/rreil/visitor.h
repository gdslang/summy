/*
 * visitor.h
 *
 *  Created on: Oct 20, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/visitor.h>
#include <summy/rreil/id/id_visitor.h>

namespace summy {
namespace rreil {

class visitor : public gdsl::rreil::visitor, public id_visitor {
protected:
  virtual void visit(ssa_id *si) {
    summy::rreil::id_visitor::visit(si);
  }
//  using gdsl::rreil::visitor::visit;
//  using id_visitor::visit;

public:
  using gdsl::rreil::visitor::_;
  using summy::rreil::id_visitor::_;

  using gdsl::rreil::visitor::_default;
  using id_visitor::_default;
};


}
}
