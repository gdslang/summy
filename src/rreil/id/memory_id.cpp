/*
 * memory_id.cpp
 *
 *  Created on: Mar 18, 2015
 *      Author: Julian Kranz
 */

#include <cppgdsl/rreil/id/id.h>
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/memory_id.h>
#include <iostream>

using namespace std;

/*
 * allocation_memory_id
 */

void summy::rreil::allocation_memory_id::put(std::ostream &out) const {
  out << "<@alloc:" << allocation_site << '>';
}

size_t summy::rreil::allocation_memory_id::subclass_counter = gdsl::rreil::id::subclass_counter++;

bool summy::rreil::allocation_memory_id::operator==(gdsl::rreil::id const &other) const {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](allocation_memory_id const *m) { equals = this->allocation_site == m->allocation_site; });
  other.accept(iv);
  return equals;
}

bool summy::rreil::allocation_memory_id::operator<(const id &other) const {
  size_t scc_me = subclass_counter;
  size_t scc_other = other.get_subclass_counter();
  if(scc_me == scc_other) {
    allocation_memory_id const &other_casted = dynamic_cast<allocation_memory_id const &>(other);
    return allocation_site < other_casted.allocation_site;
  } else
    return scc_me < scc_other;
}

std::unique_ptr<gdsl::rreil::id> summy::rreil::allocation_memory_id::copy() const {
  return std::unique_ptr<gdsl::rreil::id>(new allocation_memory_id(*this));
}

void summy::rreil::allocation_memory_id::accept(gdsl::rreil::id_visitor &v) const {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor &>(v);
  summy_v.visit(this);
}

/*
 * ptr_memory_id
 */

void summy::rreil::ptr_memory_id::put(std::ostream &out) const {
  out << '<';
  out << *inner;
  out << '>';
}

size_t summy::rreil::ptr_memory_id::subclass_counter = gdsl::rreil::id::subclass_counter++;

summy::rreil::ptr_memory_id::~ptr_memory_id() {}

bool summy::rreil::ptr_memory_id::operator==(gdsl::rreil::id const &other) const {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](ptr_memory_id const *m) { equals = *this->inner == *m->inner; });
  other.accept(iv);
  return equals;
}

bool summy::rreil::ptr_memory_id::operator<(const id &other) const {
  size_t scc_me = subclass_counter;
  size_t scc_other = other.get_subclass_counter();
  if(scc_me == scc_other) {
    ptr_memory_id const &other_casted = dynamic_cast<ptr_memory_id const &>(other);
    return *inner < *other_casted.inner;
  } else
    return scc_me < scc_other;
}

std::unique_ptr<gdsl::rreil::id> summy::rreil::ptr_memory_id::copy() const {
  return std::unique_ptr<gdsl::rreil::id>(new ptr_memory_id(*this));
}

void summy::rreil::ptr_memory_id::accept(gdsl::rreil::id_visitor &v) const {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor &>(v);
  summy_v.visit(this);
}
