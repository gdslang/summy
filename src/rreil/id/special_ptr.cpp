/*
 * special_ptr.cpp
 *
 *  Created on: Jul 27, 2015
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/special_ptr.h>

using namespace std;

void summy::rreil::special_ptr::put(std::ostream &out) const {
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

size_t summy::rreil::special_ptr::subclass_counter = gdsl::rreil::id::subclass_counter++;

summy::rreil::special_ptr::~special_ptr() {}

bool summy::rreil::special_ptr::operator==(gdsl::rreil::id const &other) const {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](special_ptr const *sp) { equals = sp->kind == kind; });
  other.accept(iv);

  return equals;
}

bool summy::rreil::special_ptr::operator<(const id &other) const {
  size_t scc_me = subclass_counter;
  size_t scc_other = other.get_subclass_counter();
  if(scc_me == scc_other) {
    special_ptr const &other_casted = dynamic_cast<special_ptr const &>(other);
    return kind < other_casted.kind;
  } else
    return scc_me < scc_other;
}

std::unique_ptr<gdsl::rreil::id> summy::rreil::special_ptr::copy() const {
  return std::unique_ptr<gdsl::rreil::id>(new special_ptr(*this));
}

void summy::rreil::special_ptr::accept(gdsl::rreil::id_visitor &v) const {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor &>(v);
  summy_v.visit(this);
}

bool summy::rreil::special_ptr::is_bad(std::shared_ptr<gdsl::rreil::id> id) {
  bool _is_bad = false;
  summy::rreil::id_visitor idv;
  idv._([&](special_ptr const *sptr) {
    switch(sptr->get_kind()) {
      case NULL_PTR: {
        break;
      }
      case BAD_PTR: {
        _is_bad = true;
        break;
      }
    }
  });
  id->accept(idv);
  return _is_bad;
}

shared_ptr<gdsl::rreil::id> summy::rreil::special_ptr::_nullptr =
  shared_ptr<gdsl::rreil::id>(new special_ptr(special_ptr_kind::NULL_PTR));
shared_ptr<gdsl::rreil::id> summy::rreil::special_ptr::badptr =
  shared_ptr<gdsl::rreil::id>(new special_ptr(special_ptr_kind::BAD_PTR));
