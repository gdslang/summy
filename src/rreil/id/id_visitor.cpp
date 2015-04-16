/*
 * id_visitor.cpp
 *
 *  Created on: Oct 28, 2014
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/rreil/id/memory_id.h>
#include <summy/rreil/id/ssa_id.h>
#include <summy/rreil/id/sm_id.h>

namespace sr = summy::rreil;

void sr::id_visitor::visit(sr::ssa_id *a) {
  if(ssa_id_callback != NULL) ssa_id_callback(a);
  else _default(a);
}

void sr::id_visitor::visit(sr::numeric_id *a) {
  if(numeric_id_callback != NULL) numeric_id_callback(a);
  else _default(a);
}

void sr::id_visitor::visit(sr::memory_id *a) {
  if(memory_id_callback != NULL) memory_id_callback(a);
  else _default(a);
}

void sr::id_visitor::visit(sr::sm_id *a) {
  if(sm_id_callback != NULL) sm_id_callback(a);
  else _default(a);
}
