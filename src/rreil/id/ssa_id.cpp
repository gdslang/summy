/*
 * ssa_id.cpp
 *
 *  Created on: Oct 20, 2014
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/ssa_id.h>
#include <summy/rreil/id/id_visitor.h>
#include <string>

using namespace std;
using namespace summy::rreil;

void ssa_id::put(std::ostream &out) {
  out << *id << "_" << version;
}

ssa_id::~ssa_id() {
  delete id;
}

void ssa_id::accept(gdsl::rreil::id_visitor &v) {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor&>(v);
  summy_v.visit(this);
}

bool summy::rreil::ssa_id::operator ==(gdsl::rreil::id &other) {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](ssa_id *aid) {
    equals = this->version == aid->version && *this->id == *aid->id;
  });
  other.accept(iv);
  return equals;
}
