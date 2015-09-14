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

size_t summy::rreil::ssa_id::subclass_counter = gdsl::rreil::id::subclass_counter++;

ssa_id::~ssa_id() {
  delete id;
}

void ssa_id::accept(gdsl::rreil::id_visitor &v) {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor&>(v);
  summy_v.visit(this);
}

bool summy::rreil::ssa_id::operator ==(gdsl::rreil::id &other) const {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](ssa_id *aid) {
    equals = this->version == aid->version && *this->id == *aid->id;
  });
  other.accept(iv);
  return equals;
}

bool summy::rreil::ssa_id::operator<(const class id &other) const {
  size_t scc_me = subclass_counter;
  size_t scc_other = other.get_subclass_counter();
  if(scc_me == scc_other) {
    ssa_id const &other_casted = dynamic_cast<ssa_id const &>(other);
    if(version < other_casted.version)
      return true;
    else if(version > other_casted.version)
      return false;
    else
      return *id < *other_casted.id;
  } else
    return scc_me < scc_other;
}
