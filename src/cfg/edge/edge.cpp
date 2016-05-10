/*
 * edge.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/rreil/copy_visitor.h>

using namespace gdsl::rreil;

/*
 * edge
 */


void cfg::edge::dot(std::ostream &stream) const {
  stream << "\"\"";
}

void cfg::edge::accept(edge_visitor &v) const {
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

cfg::stmt_edge::stmt_edge(jump_dir jd, gdsl::rreil::statement *stmt) : edge(jd) {
  summy::rreil::copy_visitor cv;
  stmt->accept(cv);
  this->stmt = cv.get_statement();
}

void cfg::stmt_edge::dot(std::ostream &stream) const {
  stream << "\"" << *stmt << "\"";
}

void cfg::stmt_edge::accept(edge_visitor &v) const {
  v.visit(this);
}

/*
 * cond_edge
 */

cfg::cond_edge::cond_edge(gdsl::rreil::sexpr *cond, bool positive) {
  this->positive = positive;

  summy::rreil::copy_visitor cv;
  cond->accept(cv);
  this->cond = cv.get_sexpr();
}

cfg::cond_edge::cond_edge(jump_dir jd, gdsl::rreil::sexpr *cond, bool positive) : edge(jd) {
  this->positive = positive;

  summy::rreil::copy_visitor cv;
  cond->accept(cv);
  this->cond = cv.get_sexpr();
}

void cfg::cond_edge::dot(std::ostream &stream) const {
  if(positive) stream << "\"" << *cond << "\"";
  else stream << "\"!(" << *cond << ")\"";
  stream << ", style=dashed, color=blue";
}

void cfg::cond_edge::accept(edge_visitor &v) const {
  v.visit(this);
}


/*
 * call_edge
 */

cfg::call_edge::call_edge(bool target_edge) : target_edge(target_edge) {
}

cfg::call_edge::call_edge(jump_dir jd, bool target_edge) : edge(jd),  target_edge(target_edge) {
}

cfg::call_edge::~call_edge() {
}

void cfg::call_edge::dot(std::ostream &stream) const {
  if(target_edge)
    stream << "\"<call>\"";
  else
    stream << "\"<expected return>\"";
  stream << ", style=dotted, color=gray";
}

void cfg::call_edge::accept(edge_visitor &v) const {
  v.visit(this);
}
