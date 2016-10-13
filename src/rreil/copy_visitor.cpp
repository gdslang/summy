/*
 * copy_visitor.cpp
 *
 *  Created on: Oct 21, 2014
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/ssa_id.h>
#include <summy/rreil/copy_visitor.h>
#include <summy/rreil/id/numeric_id.h>
#include <cppgdsl/rreil/id/id.h>
#include <summy/rreil/id/memory_id.h>
#include <summy/rreil/id/sm_id.h>

using namespace std;

void summy::rreil::copy_visitor::visit(ssa_id const *a) {
  a->get_id()->accept(*this);
  gdsl::rreil::id *id = _id.release();
  if(ssa_id_ctor != NULL)
    _id.reset(ssa_id_ctor(id, a->get_version()));
  else
    _id.reset(new ssa_id(id, a->get_version()));
}

void summy::rreil::copy_visitor::visit(numeric_id const *a) {
  if(numeric_id_ctor != NULL)
    _id.reset(numeric_id_ctor(a->get_counter(), a->get_name(), a->get_input()));
  else
    _id.reset(new numeric_id(a->get_counter(), a->get_name(), a->get_input()));
}

void summy::rreil::copy_visitor::visit(ptr_memory_id const *a) {
  a->get_id()->accept(*this);
  gdsl::rreil::id *id = _id.release();
  if(ptr_memory_id_ctor != NULL)
    _id.reset(ptr_memory_id_ctor(shared_ptr<gdsl::rreil::id>(id)));
  else
    _id.reset(new ptr_memory_id(shared_ptr<gdsl::rreil::id>(id)));
}

void summy::rreil::copy_visitor::visit(allocation_memory_id const *a) {
  if(allocation_memory_id_ctor != NULL)
    _id.reset(allocation_memory_id_ctor(a->get_allocation_site()));
  else
    _id.reset(new allocation_memory_id(a->get_allocation_site()));
}

void summy::rreil::copy_visitor::visit(sm_id *a) {
  if(sm_id_ctor != NULL)
    _id.reset(sm_id_ctor(a->get_symbol(), a->get_address()));
  else
    _id.reset(new sm_id(a->get_symbol(), a->get_address()));
}
