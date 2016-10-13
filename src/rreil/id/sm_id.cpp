/*
 * sm_id.cpp
 *
 *  Created on: Apr 16, 2015
 *      Author: Julian Kranz
 */

#include <summy/rreil/id/sm_id.h>
#include <summy/rreil/id/id_visitor.h>

using namespace std;

void summy::rreil::sm_id::put(std::ostream &out) const {
  out << "<!" << symbol << "@" << hex << address << dec << ">";
}

size_t summy::rreil::sm_id::subclass_counter = gdsl::rreil::id::subclass_counter++;

summy::rreil::sm_id::~sm_id() {}

bool summy::rreil::sm_id::operator==(gdsl::rreil::id const &other) const {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](sm_id const *sid) { equals = this->symbol == sid->symbol && this->address == sid->address; });
  other.accept(iv);
  return equals;
}

bool summy::rreil::sm_id::operator<(const id &other) const {
  size_t scc_me = subclass_counter;
  size_t scc_other = other.get_subclass_counter();
  if(scc_me == scc_other) {
    sm_id const &other_casted = dynamic_cast<sm_id const &>(other);
    if(address < other_casted.address)
      return true;
    else if(address > other_casted.address)
      return false;
    else
      return symbol.compare(other_casted.symbol) < 0;
  } else
    return scc_me < scc_other;
}

void summy::rreil::sm_id::accept(gdsl::rreil::id_visitor &v) const {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor &>(v);
  summy_v.visit(this);
}

std::shared_ptr<gdsl::rreil::id> summy::rreil::sm_id::from_symbol(analysis::symbol symb) {
  return shared_ptr<gdsl::rreil::id>(new sm_id(symb.symbol_name, symb.address));
}
