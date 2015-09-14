/*
 * memory_id.cpp
 *
 *  Created on: Mar 18, 2015
 *      Author: Julian Kranz
 */

#include <cppgdsl/rreil/id/id.h>
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/memory_id.h>

void summy::rreil::memory_id::put(std::ostream &out) {
  out << '<';
  for(size_t i = 0; i < deref; i++)
    out << "*";
  out << *inner;
  out << '>';
}

size_t summy::rreil::memory_id::subclass_counter = gdsl::rreil::id::subclass_counter++;

summy::rreil::memory_id::~memory_id() {
}

bool summy::rreil::memory_id::operator ==(gdsl::rreil::id &other) const {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](memory_id *m) {
    equals = this->deref == m->deref && *this->inner == *m->inner;
  });
  other.accept(iv);
  return equals;
}

bool summy::rreil::memory_id::operator<(const id &other) const {
  size_t scc_me = subclass_counter;
  size_t scc_other = other.get_subclass_counter();
  if(scc_me == scc_other) {
    memory_id const &other_casted = dynamic_cast<memory_id const &>(other);
    if(deref < other_casted.deref)
      return true;
    else if(deref > other_casted.deref)
      return false;
    else
      return *inner < *other_casted.inner;
  } else
    return scc_me < scc_other;
}

void summy::rreil::memory_id::accept(gdsl::rreil::id_visitor &v) {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor&>(v);
  summy_v.visit(this);
}
