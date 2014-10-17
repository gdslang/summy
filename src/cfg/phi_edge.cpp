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

cfg::phi_edge::phi_edge(assignments_t assignments) {
  copy_visitor cv;
  for(auto ass : assignments) {
    variable *lhs;
    ass.lhs->accept(cv);
    lhs = cv.get_variable();
    variable *rhs;
    ass.rhs->accept(cv);
    rhs = cv.get_variable();
    this->assignments.push_back(phi_assign(lhs, rhs, ass.size));
  }
}

cfg::phi_edge::~phi_edge() {
  for(auto ass : assignments) {
    delete ass.lhs;
    delete ass.rhs;
  }
}

void cfg::phi_edge::dot(std::ostream &stream) {
  stream << "\"";
  for(auto ass : assignments)
    stream << *ass.lhs << " =:" << ass.size << " " << *ass.rhs << endl;
  stream << "\"";
  stream << ", style=bold, color=green";
}

void cfg::phi_edge::accept(edge_visitor &v) {
  v.visit(this);
}
