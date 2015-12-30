/*
 * analysis_id.cpp
 *
 *  Created on: Mar 12, 2015
 *      Author: Julian Kranz
 */

#include <cppgdsl/rreil/id/id.h>
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/numeric_id.h>

using namespace summy::rreil;
using namespace std;

void summy::rreil::numeric_id::put(std::ostream &out) {
  out << '#';
  if(name)
    out << name.value() << (input.value() ? "i" : "");
  else
    out << counter;
}

size_t summy::rreil::numeric_id::subclass_counter = gdsl::rreil::id::subclass_counter++;

summy::rreil::numeric_id::~numeric_id() {}

bool summy::rreil::numeric_id::operator==(gdsl::rreil::id &other) const {
  bool equals = false;
  summy::rreil::id_visitor iv;
  iv._([&](numeric_id *aid) { equals = this->counter == aid->counter; });
  other.accept(iv);
  return equals;
}

bool summy::rreil::numeric_id::operator<(const id &other) const {
  size_t scc_me = subclass_counter;
  size_t scc_other = other.get_subclass_counter();
  if(scc_me == scc_other) {
    numeric_id const &other_casted = dynamic_cast<numeric_id const &>(other);
    return counter < other_casted.counter;
  } else
    return scc_me < scc_other;
}

void summy::rreil::numeric_id::accept(gdsl::rreil::id_visitor &v) {
  auto &summy_v = dynamic_cast<summy::rreil::id_visitor &>(v);
  summy_v.visit(this);
}

std::shared_ptr<gdsl::rreil::id> summy::rreil::numeric_id::generate(
  std::experimental::optional<std::string> name, std::experimental::optional<bool> input) {
  static int_t counter = 0;
  return shared_ptr<gdsl::rreil::id>(new numeric_id(counter++, name, input));
}

std::shared_ptr<gdsl::rreil::id> summy::rreil::numeric_id::generate(
  std::string reg_name, int64_t offset, size_t size, bool input) {
  string pos;
  if(offset == 0)
    pos = "";
  else if(offset == size)
    pos = "H";
  else
    return generate();

  switch(size) {
    case 64: {
      return generate(reg_name + "_q" + pos, input);
    }
    case 32: {
      return generate(reg_name + "_l" + pos, input);
    }
    case 16: {
      return generate(reg_name + "_w" + pos, input);
    }
    case 8: {
      return generate(reg_name + "_b" + pos, input);
    }
  }

  return generate();
}
