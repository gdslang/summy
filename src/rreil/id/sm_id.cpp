/*
 * sm_id.cpp
 *
 *  Created on: Apr 16, 2015
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/sm_id.h>
#include <summy/rreil/id/id_visitor.h>

using namespace std;

void summy::rreil::sm_id::put(std::ostream &out) {
  out << "<!" << symbol << "@" << hex << address << dec << ">";
}

summy::rreil::sm_id::~sm_id() {}

bool summy::rreil::sm_id::operator==(gdsl::rreil::id &other) {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](sm_id *sid) { equals = this->symbol == sid->symbol && this->address == sid->address; });
  other.accept(iv);
  return equals;
}

void summy::rreil::sm_id::accept(gdsl::rreil::id_visitor &v) {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor &>(v);
  summy_v.visit(this);
}

std::shared_ptr<gdsl::rreil::id> summy::rreil::sm_id::from_symbol(analysis::symbol symb) {
  return shared_ptr<gdsl::rreil::id>(new sm_id(symb.symbol_name, symb.address));
}
