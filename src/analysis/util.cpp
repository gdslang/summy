/*
 * util.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <cppgdsl/rreil/rreil.h>
#include <summy/analysis/domain_state.h>
#include <summy/analysis/util.h>
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/memory_id.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/rreil/id/sm_id.h>
#include <summy/rreil/id/special_ptr.h>
#include <summy/rreil/id/ssa_id.h>
#include <typeindex>

#include <iostream>
#include <string>

using namespace std;

using namespace gdsl::rreil;

namespace sr = summy::rreil;

string analysis::print_id_no_version(gdsl::rreil::id const &x) {
  string str;
  sr::id_visitor iv;
  iv._([&](sr::ssa_id const *si) { str = si->get_id().to_string(); });
  iv._default([&](id const *_id) { str = _id->to_string(); });
  x.accept(iv);
  return str;
}

bool analysis::id_less_no_version::operator()(
  std::shared_ptr<gdsl::rreil::id> const &a, std::shared_ptr<gdsl::rreil::id> const &b) const {
  return print_id_no_version(*a).compare(print_id_no_version(*b)) < 0;
}

bool analysis::id_less::operator()(
  std::shared_ptr<gdsl::rreil::id> const &a, std::shared_ptr<gdsl::rreil::id> const &b) const {
  return *a < *b;
}

bool analysis::id_less::operator()(
  const gdsl::rreil::id &a, const std::shared_ptr<gdsl::rreil::id> &b) const {
  return a < *b;
}

bool analysis::id_less::operator()(
  const std::shared_ptr<gdsl::rreil::id> &a, const gdsl::rreil::id &b) const {
  return *a < b;
}

bool analysis::id_less::operator()(const gdsl::rreil::id &a, const gdsl::rreil::id &b) const {
  return a < b;
}
