/*
 * phi_edge.cpp
 *
 *  Created on: Oct 17, 2014
 *      Author: Julian Kranz
 */
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/cfg/edge/phi_edge.h>
#include <summy/rreil/copy_visitor.h>
#include <iostream>

using namespace gdsl::rreil;
using namespace std;

cfg::phi_assign::phi_assign(gdsl::rreil::variable *lhs, gdsl::rreil::variable *rhs, int_t size) : size(size) {
  summy::rreil::copy_visitor cv;
  lhs->accept(cv);
  this->lhs = cv.retrieve_variable().release();
  rhs->accept(cv);
  this->rhs = cv.retrieve_variable().release();
}

cfg::phi_assign::phi_assign(const phi_assign &a) : phi_assign(a.lhs, a.rhs, a.size) {}

cfg::phi_assign::~phi_assign() {
  delete this->lhs;
  delete this->rhs;
}

cfg::phi_edge::phi_edge(assignments_t assignments, phi_memory memory) : assignments(assignments), memory(memory) {}

cfg::phi_edge::phi_edge(jump_dir jd, assignments_t assignments, phi_memory memory)
    : edge(jd), assignments(assignments), memory(memory) {}

cfg::phi_edge::~phi_edge() {}

void cfg::phi_edge::dot(std::ostream &stream) const {
  stream << "\"";
  for(auto ass : assignments)
    stream << ass.get_lhs() << " =:" << ass.get_size() << " " << ass.get_rhs() << endl;
  stream << "memory_" << memory.to << " =: memory_" << memory.from << endl;
  stream << "\"";
  stream << ", style=bold, color=green";
}

void cfg::phi_edge::accept(edge_visitor &v) const {
  v.visit(this);
}
