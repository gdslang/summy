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
  auto id = std::move(_id);
  if(ssa_id_ctor != NULL)
    _id = ssa_id_ctor(std::move(id), a->get_version());
  else
    _id = make_unique<ssa_id>(std::move(id).release(), a->get_version());
}

void summy::rreil::copy_visitor::visit(numeric_id const *a) {
  if(numeric_id_ctor != NULL)
    _id = numeric_id_ctor(a->get_counter(), a->get_name(), a->get_input());
  else
    _id = std::unique_ptr<numeric_id>(new numeric_id(a->get_counter(), a->get_name(), a->get_input()));
}

void summy::rreil::copy_visitor::visit(ptr_memory_id const *a) {
  a->get_id().accept(*this);
  auto id = std::move(_id);
  if(ptr_memory_id_ctor != NULL)
    _id = ptr_memory_id_ctor(std::move(id));
  else
    _id = make_unique<ptr_memory_id>(std::move(id));
}

void summy::rreil::copy_visitor::visit(allocation_memory_id const *a) {
  if(allocation_memory_id_ctor != NULL)
    _id = allocation_memory_id_ctor(a->get_allocation_site());
  else
    _id = make_unique<allocation_memory_id>(a->get_allocation_site());
}

void summy::rreil::copy_visitor::visit(sm_id *a) {
  if(sm_id_ctor != NULL)
    _id = sm_id_ctor(a->get_symbol(), a->get_address());
  else
    _id = make_unique<sm_id>(a->get_symbol(), a->get_address());
}
