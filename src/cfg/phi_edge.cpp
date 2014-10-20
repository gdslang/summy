/*
 * phi_edge.cpp
 *
 *  Created on: Oct 17, 2014
 *      Author: Julian Kranz
 */
#include <summy/cfg/phi_edge.h>
#include <summy/cfg/edge_visitor.h>
#include <iostream>

#include <cppgdsl/rreil/copy_visitor.h>

using namespace gdsl::rreil;
using namespace std;

cfg::phi_assign::phi_assign(gdsl::rreil::variable *lhs, gdsl::rreil::variable *rhs, int_t size) : size(size) {
  copy_visitor cv;
  lhs->accept(cv);
  this->lhs = cv.get_variable();
  rhs->accept(cv);
  this->rhs = cv.get_variable();

//  cout << "this: " << this->lhs << " /-> id: " << this->lhs->get_id() << endl;
//  cout << "other: " << lhs << " /-> id: " << lhs->get_id() << endl;
}

cfg::phi_assign::phi_assign(const phi_assign &a) : phi_assign(a.lhs, a.rhs, a.size) {
}

cfg::phi_assign::~phi_assign() {
  delete this->lhs;
  delete this->rhs;
}

cfg::phi_edge::phi_edge(assignments_t assignments) : assignments(assignments) {
}

cfg::phi_edge::~phi_edge() {
}

void cfg::phi_edge::dot(std::ostream &stream) {
  stream << "\"";
  for(auto ass : assignments)
    stream << *ass.get_lhs() << " =:" << ass.get_size() << " " << *ass.get_rhs() << endl;
  stream << "\"";
  stream << ", style=bold, color=green";
}

void cfg::phi_edge::accept(edge_visitor &v) {
  v.visit(this);
}
