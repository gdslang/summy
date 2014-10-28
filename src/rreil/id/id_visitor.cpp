/*
 * id_visitor.cpp
 *
 *  Created on: Oct 28, 2014
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/ssa_id.h>

namespace sr = summy::rreil;

void sr::id_visitor::visit(sr::ssa_id *a) {
  if(ssa_id_callback != NULL) ssa_id_callback(a);
  else _default(a);
}
