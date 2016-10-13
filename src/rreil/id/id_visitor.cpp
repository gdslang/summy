/*
 * id_visitor.cpp
 *
 *  Created on: Oct 28, 2014
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/memory_id.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/rreil/id/ssa_id.h>
#include <summy/rreil/id/sm_id.h>
#include <summy/rreil/id/special_ptr.h>

namespace sr = summy::rreil;

void sr::id_visitor::visit(sr::ssa_id const *a) {
  if(ssa_id_callback != NULL)
    ssa_id_callback(a);
  else
    _default(a);
}

void sr::id_visitor::visit(sr::numeric_id const *a) {
  if(numeric_id_callback != NULL)
    numeric_id_callback(a);
  else
    _default(a);
}

void sr::id_visitor::visit(sr::ptr_memory_id const *a) {
  if(ptr_memory_id_callback != NULL)
    ptr_memory_id_callback(a);
  else
    _default(a);
}

void sr::id_visitor::visit(sr::allocation_memory_id const *a) {
  if(allocation_memory_id_callback != NULL)
    allocation_memory_id_callback(a);
  else
    _default(a);
}


void sr::id_visitor::visit(sr::sm_id const *a) {
  if(sm_id_callback != NULL)
    sm_id_callback(a);
  else
    _default(a);
}

void sr::id_visitor::visit(sr::special_ptr const *a) {
  if(spcial_ptr_callback != NULL)
    spcial_ptr_callback(a);
  else
    _default(a);
}
