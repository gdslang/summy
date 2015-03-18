/*
 * memory_id.cpp
 *
 *  Created on: Mar 18, 2015
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/memory_id.h>

void summy::rreil::memory_id::put(std::ostream &out) {
  for(size_t i = 0; i < deref; i++)
    out << "*";
  out << *inner;
}

summy::rreil::memory_id::~memory_id() {
}

bool summy::rreil::memory_id::operator ==(gdsl::rreil::id &other) {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](memory_id *m) {
    equals = this->deref == m->deref && *this->inner == *m->inner;
  });
  other.accept(iv);
  return equals;
}

void summy::rreil::memory_id::accept(gdsl::rreil::id_visitor &v) {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor&>(v);
  summy_v.visit(this);
}
