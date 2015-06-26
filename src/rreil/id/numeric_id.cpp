/*
 * analysis_id.cpp
 *
 *  Created on: Mar 12, 2015
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/numeric_id.h>

using namespace summy::rreil;
using namespace std;

void summy::rreil::numeric_id::put(std::ostream &out) {
  out << "#" << counter;
}

summy::rreil::numeric_id::~numeric_id() {
}

bool summy::rreil::numeric_id::operator ==(gdsl::rreil::id &other) {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](numeric_id *aid) {
    equals = this->counter == aid->counter;
  });
  other.accept(iv);
  return equals;
}

void summy::rreil::numeric_id::accept(gdsl::rreil::id_visitor &v) {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor&>(v);
  summy_v.visit(this);
}

std::shared_ptr<gdsl::rreil::id> summy::rreil::numeric_id::generate() {
  static int_t counter = 66;
  return shared_ptr<gdsl::rreil::id>(new numeric_id(counter++));
}
