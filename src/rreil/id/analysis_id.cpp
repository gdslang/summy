/*
 * analysis_id.cpp
 *
 *  Created on: Mar 12, 2015
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/analysis_id.h>
#include <summy/rreil/id/id_visitor.h>

using namespace summy::rreil;
using namespace std;

void summy::rreil::analysis_id::put(std::ostream &out) {
  out << "~" << numeric_id;
}

summy::rreil::analysis_id::~analysis_id() {
}

bool summy::rreil::analysis_id::operator ==(gdsl::rreil::id &other) {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](analysis_id *aid) {
    equals = this->numeric_id == aid->numeric_id;
  });
  other.accept(iv);
  return equals;
}

void summy::rreil::analysis_id::accept(gdsl::rreil::id_visitor &v) {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor&>(v);
  summy_v.visit(this);
}

std::shared_ptr<gdsl::rreil::id> summy::rreil::analysis_id::generate() {
  static size_t counter = 0;
  return shared_ptr<gdsl::rreil::id>(new analysis_id(counter++));
}
