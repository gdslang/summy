/*
 * copy_visitor.cpp
 *
 *  Created on: Oct 21, 2014
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/ssa_id.h>
#include <summy/rreil/copy_visitor.h>
#include <cppgdsl/rreil/id/id.h>

void summy::rreil::copy_visitor::visit(ssa_id *a) {
  a->get_id()->accept(*this);
  gdsl::rreil::id *id = _id;
  if(ssa_id_ctor != NULL) _id = ssa_id_ctor(id, a->get_version());
  else _id = new ssa_id(id, a->get_version());
}
