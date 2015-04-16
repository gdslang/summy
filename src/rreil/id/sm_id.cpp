/*
 * sm_id.cpp
 *
 *  Created on: Apr 16, 2015
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/sm_id.h>

using namespace std;

void summy::rreil::sm_id::put(std::ostream &out) {
  out << "<!" << symbol << ">";
}

summy::rreil::sm_id::~sm_id() {
}

bool summy::rreil::sm_id::operator ==(gdsl::rreil::id &other) {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](sm_id *sid) {
    equals = this->symbol == sid->symbol;
  });
  other.accept(iv);
  return equals;
}

void summy::rreil::sm_id::accept(gdsl::rreil::id_visitor &v) {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor&>(v);
  summy_v.visit(this);
}

std::shared_ptr<gdsl::rreil::id> summy::rreil::sm_id::from_symbol(std::string symbol) {
  return shared_ptr<gdsl::rreil::id>(new sm_id(symbol));
}
