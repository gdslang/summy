/*
 * util.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/util.h>
#include <summy/rreil/id/id_visitor.h>
#include <cppgdsl/rreil/rreil.h>
#include <summy/rreil/id/ssa_id.h>
#include <assert.h>
#include <summy/analysis/domain_state.h>
#include <summy/rreil/id/memory_id.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/rreil/id/sm_id.h>
#include <summy/rreil/id/special_ptr.h>
#include <typeindex>

#include <iostream>
#include <string>

using namespace std;
using namespace std::experimental;
using namespace gdsl::rreil;

namespace sr = summy::rreil;

string analysis::print_id_no_version(std::shared_ptr<gdsl::rreil::id> x) {
  string str;
  sr::id_visitor iv;
  iv._([&](sr::ssa_id *si) { str = si->get_id()->to_string(); });
  iv._default([&](id *_id) { str = _id->to_string(); });
  x->accept(iv);
  return str;
}

bool analysis::id_less_no_version::operator()(
  std::shared_ptr<gdsl::rreil::id> a, std::shared_ptr<gdsl::rreil::id> b) const {

//  cout << "-------" << endl;
  if(*a < *b && *b < *a)
    cout << *a << " <> " << *b << endl;
  if(!(*a < *b) && !(*b < *a))
    if(print_id_no_version(a).compare(print_id_no_version(b)) != 0)
    cout << *a << " <==> " << *b << endl;

  return *a < *b;
}
