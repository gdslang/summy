/*
 * special_ptr.cpp
 *
 *  Created on: Jul 27, 2015
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/special_ptr.h>
#include <summy/rreil/id/id_visitor.h>

using namespace std;

void summy::rreil::special_ptr::put(std::ostream &out) {
  out << "<";
  switch(kind) {
    case NULL_PTR: {
      out << "null";
      break;
    }
    case BAD_PTR: {
      out << "bad";
      break;
    }
  }
  out << ">";
}

summy::rreil::special_ptr::~special_ptr() {}

bool summy::rreil::special_ptr::operator==(gdsl::rreil::id &other) const {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](special_ptr *sp) { equals = sp->kind == kind; });
  other.accept(iv);

  return equals;
}

void summy::rreil::special_ptr::accept(gdsl::rreil::id_visitor &v) {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor &>(v);
  summy_v.visit(this);
}

shared_ptr<gdsl::rreil::id> summy::rreil::special_ptr::_nullptr =
  shared_ptr<gdsl::rreil::id>(new special_ptr(special_ptr_kind::NULL_PTR));
shared_ptr<gdsl::rreil::id> summy::rreil::special_ptr::badptr =
  shared_ptr<gdsl::rreil::id>(new special_ptr(special_ptr_kind::BAD_PTR));
