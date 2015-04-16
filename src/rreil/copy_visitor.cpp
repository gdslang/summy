/*
 * copy_visitor.cpp
 *
 *  Created on: Oct 21, 2014
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/ssa_id.h>
#include <summy/rreil/copy_visitor.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/rreil/id/memory_id.h>
#include <cppgdsl/rreil/id/id.h>
#include <summy/rreil/id/sm_id.h>

using namespace std;

void summy::rreil::copy_visitor::visit(ssa_id *a) {
  a->get_id()->accept(*this);
  gdsl::rreil::id *id = _id;
  if(ssa_id_ctor != NULL) _id = ssa_id_ctor(id, a->get_version());
  else _id = new ssa_id(id, a->get_version());
}

void summy::rreil::copy_visitor::visit(numeric_id *a) {
  if(numeric_id_ctor != NULL) _id = numeric_id_ctor(a->get_counter());
  else _id = new numeric_id(a->get_counter());
}

void summy::rreil::copy_visitor::visit(memory_id *a) {
  a->get_id()->accept(*this);
  gdsl::rreil::id *id = _id;
  if(memory_id_ctor != NULL) _id = memory_id_ctor(a->get_deref(), shared_ptr<gdsl::rreil::id>(id));
  else _id = new memory_id(a->get_deref(), shared_ptr<gdsl::rreil::id>(id));
}

void summy::rreil::copy_visitor::visit(sm_id *a) {
  if(sm_id_ctor != NULL) _id = sm_id_ctor(a->get_symbol());
  else _id = new sm_id(a->get_symbol());
}
