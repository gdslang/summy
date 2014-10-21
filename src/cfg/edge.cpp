/*
 * edge.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#include <summy/cfg/edge.h>
#include <summy/cfg/edge_visitor.h>
#include <summy/rreil/copy_visitor.h>

using namespace gdsl::rreil;

/*
 * edge
 */

void cfg::edge::dot(std::ostream &stream) {
  stream << "\"\"";
}

void cfg::edge::accept(edge_visitor &v) {
  v.visit(this);
}

/*
 * stmt_edge
 */

cfg::stmt_edge::stmt_edge(gdsl::rreil::statement *stmt) {
  summy::rreil::copy_visitor cv;
  stmt->accept(cv);
  this->stmt = cv.get_statement();
}

void cfg::stmt_edge::dot(std::ostream &stream) {
  stream << "\"" << *stmt << "\"";
}

void cfg::stmt_edge::accept(edge_visitor &v) {
  v.visit(this);
}

/*
 * cond_edge
 */

cfg::cond_edge::cond_edge(gdsl::rreil::sexpr *cond, bool positive) {
  this->positive = positive;

  copy_visitor cv;
  cond->accept(cv);
  this->cond = cv.get_sexpr();
}

void cfg::cond_edge::dot(std::ostream &stream) {
  if(positive) stream << "\"" << *cond << "\"";
  else stream << "\"!(" << *cond << ")\"";
  stream << ", style=dashed, color=blue";
}

void cfg::cond_edge::accept(edge_visitor &v) {
  v.visit(this);
}
