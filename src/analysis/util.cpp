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

std::vector<std::experimental::optional<bool>> analysis::id_less_no_version::less;
summy::rreil::id_visitor analysis::id_less_no_version::iv_a;
summy::rreil::id_visitor analysis::id_less_no_version::iv_b = construct_visitors();

summy::rreil::id_visitor analysis::id_less_no_version::construct_visitors() {
  //  cout << "a: " << *a << ", b: " << *b << endl;

  sr::id_visitor _iv_b;

  iv_a._([&](sr::memory_id *mi_a) {
    _iv_b._([&, mi_a](sr::memory_id *mi_b) {
      if(mi_a->get_deref() < mi_b->get_deref())
        less.back() = true;
      else if(mi_a->get_deref() > mi_b->get_deref())
        less.back() = false;
      else
        less.back() = id_less_no_version()(mi_a->get_id(), mi_b->get_id());
    });
  });
  iv_a._([&](sr::numeric_id *ni_a) {
    _iv_b._([&, ni_a](sr::numeric_id *ni_b) {
      //      cout << *ni_a << ", " << *ni_b << " ~~~ " << ni_a->get_counter() << "   " << ni_b->get_counter() << endl;
      if(ni_a->get_counter() < ni_b->get_counter())
        less.back() = true;
      else
        less.back() = false;
    });
  });
  iv_a._([&](sr::sm_id *si_a) {
    _iv_b._([&, si_a](sr::sm_id *si_b) {
      if(si_a->get_address() < si_b->get_address())
        less.back() = true;
      else if(si_a->get_address() > si_b->get_address())
        less.back() = false;
      else
        less.back() = si_a->get_symbol().compare(si_b->get_symbol()) < 0;
    });
  });
  iv_a._([&](sr::special_ptr *sp_a) {
    _iv_b._([&, sp_a](sr::special_ptr *sp_b) {
      if(sp_a->get_kind() < sp_b->get_kind())
        less.back() = true;
      else
        less.back() = false;
    });
  });
  iv_a._([&](sr::ssa_id *si_a) {
    _iv_b._([&, si_a](sr::ssa_id *si_b) {
      id_shared_t id_a = shared_ptr<gdsl::rreil::id>(si_a->get_id(), [](auto *x) {});
      id_shared_t id_b = shared_ptr<gdsl::rreil::id>(si_b->get_id(), [](auto *x) {});
      less.back() = id_less_no_version()(id_a, id_b);
    });
  });
  iv_a._([&](
    arch_id *ai_a) { _iv_b._([&, ai_a](arch_id *ai_b) { less.back() = ai_a->get_name().compare(ai_b->get_name()) < 0; }); });
  iv_a._([&](shared_id *s_a) { _iv_b._([&, s_a](shared_id *s_b) { less.back() = s_a->get_inner() < s_b->get_inner(); }); });
  iv_a._([&](_virtual *v_a) { _iv_b._([&, v_a](_virtual *v_b) { less.back() = v_a->get_t() < v_b->get_t(); }); });
  iv_a._default([&](id *_id) { assert(false); });

  return _iv_b;
}

string analysis::print_id_no_version(std::shared_ptr<gdsl::rreil::id> x) {
  string str;
  sr::id_visitor iv;
  iv._([&](sr::ssa_id *si) { str = si->get_id()->to_string(); });
  iv._default([&](id *_id) { str = _id->to_string(); });
  x->accept(iv);
  return str;
}

size_t id_type_id(std::shared_ptr<gdsl::rreil::id> x) {
  optional<size_t> result;
  sr::id_visitor iv;
  iv._([&](sr::memory_id *mi_a) { result = 0; });
  iv._([&](sr::numeric_id *ni_a) { result = 1; });
  iv._([&](sr::sm_id *si_a) { result = 2; });
  iv._([&](sr::special_ptr *sp_a) { result = 3; });
  iv._([&](sr::ssa_id *si_a) { result = 4; });
  iv._([&](arch_id *ai_a) { result = 5; });
  iv._([&](shared_id *s_a) { result = 6; });
  iv._([&](_virtual *v_a) { result = 7; });
  iv._default([&](id *_id) { assert(false); });
  x->accept(iv);
  return result.value();
}

bool analysis::id_less_no_version::foo(std::shared_ptr<gdsl::rreil::id> a, std::shared_ptr<gdsl::rreil::id> b) const {

  less.push_back(nullopt);

//  cout << "before: " << less.operator bool() << endl;

  a->accept(iv_a);
  b->accept(iv_b);

//  cout << "after: " << less.operator bool() << endl;

  auto _less = less.back();
  less.pop_back();

  return _less.value_or(id_type_id(a) < id_type_id(b));
}

bool analysis::id_less_no_version::operator()(
  std::shared_ptr<gdsl::rreil::id> a, std::shared_ptr<gdsl::rreil::id> b) const {

    cout << "-------" << endl;
    if(foo(a, b) && foo(b, a))
      cout << *a << " <> " << *b << endl;
    if(!foo(a, b) && !foo(b, a))
      cout << *a << " == " << *b << endl;

  return foo(a, b);
}
