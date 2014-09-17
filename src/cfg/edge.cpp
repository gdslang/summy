/*
 * edge.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#include <summy/cfg/edge.h>

/*
 * stmt_edge
 */

void cfg::stmt_edge::dot(std::ostream &stream) {
  stream << "\"" << *stmt << "\"";
}

void cfg::stmt_edge::accept(edge_visitor& v) {
  v.visit(this);
}

/*
 * cond_edge
 */

void cfg::cond_edge::dot(std::ostream &stream) {
  if(positive) stream << "\"" << *cond << "\"";
  else stream << "\"!(" << *cond << ")\"";
}

void cfg::cond_edge::accept(edge_visitor& v) {
  v.visit(this);
}
