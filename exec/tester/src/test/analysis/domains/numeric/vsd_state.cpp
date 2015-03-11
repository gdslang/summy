/*
 * vsd_state.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: Julian Kranz
 */

#include <gtest/gtest.h>
#include <iostream>
#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/test/analysis/domains/numeric_api_builder.h>
#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_finite.h>
#include <summy/value_set/vs_open.h>
#include <bjutil/autogc.h>

using namespace summy;
using namespace analysis::api;
using namespace analysis::value_sets;
using namespace analysis;
using namespace std;

class vsd_state_test: public ::testing::Test {
protected:

  vsd_state_test() {
  }

  virtual ~vsd_state_test() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

//template<typename T>
//static void repl(T *(&o), T *n) {
//  delete o;
//  o = n;
//}

TEST_F(vsd_state_test, SimpleAssignments) {
  autogc gc;
  nab n(gc);

  auto a = rreil_builder::temporary("a");
  auto b = rreil_builder::temporary("b");
  auto c = rreil_builder::temporary("c");
  auto d = rreil_builder::temporary("d");
  auto e = rreil_builder::temporary("e");
  auto f = rreil_builder::temporary("f");
  auto g = rreil_builder::temporary("g");

  auto assign_state = [&](auto &s, auto &a, auto &lin) {
    s = gc(s->assign(n.var(a), n.expr(lin)));
  };

  vsd_state *s = gc(new vsd_state());
  {
    auto assign = [&](auto a, auto lin) {
      assign_state(s, a, lin);
    };

    assign(a, nab_lin(1));
    assign(b, nab_lin(vs_shared_t(new vs_finite( { 2, 3, 4 }))));
    assign(c, a + (2 * b + 3));
    assign(d, nab_lin(vs_shared_t(new vs_open(UPWARD, 3))));
    assign(e, (a + (-c)) + 5 * d);
    assign(f, a + e);
    assign(g, a + (-c));
  }

  vsd_state *comp = gc(new vsd_state());
  {
    auto assign = [&](auto a, auto lin) {
      assign_state(comp, a, lin);
    };

    assign(a, nab_lin(1));
    assign(b, vs_shared_t(new vs_finite( { 2, 3, 4 })));
    assign(c, vs_shared_t(new vs_finite( { 8, 10, 12 })));
    assign(d, nab_lin(vs_shared_t(new vs_open(UPWARD, 3))));
    assign(e, nab_lin(vs_shared_t(new vs_open(UPWARD, 4))));
    assign(f, nab_lin(vs_shared_t(new vs_open(UPWARD, 5))));
    assign(g, vs_shared_t(new vs_finite( { -11, -9, -7 })));
  }

  ASSERT_EQ(*s, *comp);
}

TEST_F(vsd_state_test, StateComparison) {
  autogc gc;
  nab n(gc);

  auto a = rreil_builder::temporary("a");
  auto b = rreil_builder::temporary("b");
  auto c = rreil_builder::temporary("c");
  auto d = rreil_builder::temporary("d");
  auto e = rreil_builder::temporary("e");
  auto f = rreil_builder::temporary("f");
  auto g = rreil_builder::temporary("g");
  auto h = rreil_builder::temporary("h");
  auto i = rreil_builder::temporary("i");

  auto assign_state = [&](auto &s, auto &a, auto &lin) {
    s = gc(s->assign(n.var(a), n.expr(lin)));
  };

  vsd_state *greater = gc(new vsd_state());
  {
    auto assign = [&](auto a, auto lin) {
      assign_state(greater, a, lin);
    };

    assign(a, nab_lin(1));
    assign(b, vs_shared_t(new vs_finite( { 2, 3, 4, 90 })));
    assign(c, vs_shared_t(new vs_finite( { 8, 10, 12 })));
    assign(d, nab_lin(vs_shared_t(new vs_open(UPWARD, 1))));
    assign(e, nab_lin(vs_shared_t(new vs_open(UPWARD, 4))));
    assign(f, nab_lin(vs_shared_t(new vs_open(UPWARD, 5))));
    assign(i, value_set::top);
  }

  vsd_state *smaller = gc(new vsd_state());
  {
    auto assign = [&](auto a, auto lin) {
      assign_state(smaller, a, lin);
    };

    assign(a, nab_lin(1));
    assign(b, vs_shared_t(new vs_finite( { 2, 3, 4 })));
    assign(c, vs_shared_t(new vs_finite( { 8, 10, 12 })));
    assign(d, nab_lin(vs_shared_t(new vs_open(UPWARD, 3))));
    assign(e, nab_lin(vs_shared_t(new vs_open(UPWARD, 4))));
    assign(f, nab_lin(vs_shared_t(new vs_open(UPWARD, 5))));
    assign(g, vs_shared_t(new vs_finite( { -11, -9, -7 })));
    assign(h, value_set::top);
  }

  ASSERT_GE(*greater, *smaller);
  ASSERT_FALSE(*greater <= *smaller);
}

TEST_F(vsd_state_test, StateJoin) {
  autogc gc;
  nab n(gc);

  auto a = rreil_builder::temporary("a");
  auto b = rreil_builder::temporary("b");
  auto c = rreil_builder::temporary("c");
  auto d = rreil_builder::temporary("d");
  auto e = rreil_builder::temporary("e");
  auto f = rreil_builder::temporary("f");
  auto g = rreil_builder::temporary("g");
  auto h = rreil_builder::temporary("h");
  auto i = rreil_builder::temporary("i");

  auto assign_state = [&](auto &s, auto &a, auto &lin) {
    s = gc(s->assign(n.var(a), n.expr(lin)));
  };

  vsd_state *sa = gc(new vsd_state());
  {
    auto assign = [&](auto a, auto lin) {
      assign_state(sa, a, lin);
    };

    assign(a, nab_lin(1));
    assign(b, vs_shared_t(new vs_finite( { 2, 3, 4, 90 })));
    assign(c, vs_shared_t(new vs_finite( { 8, 10, 12, 13 })));
    assign(d, nab_lin(vs_shared_t(new vs_open(UPWARD, 1))));
    assign(e, nab_lin(vs_shared_t(new vs_open(UPWARD, 4))));
    assign(f, nab_lin(vs_shared_t(new vs_open(DOWNWARD, -5))));
    assign(i, value_set::top);
  }

  vsd_state *sb = gc(new vsd_state());
  {
    auto assign = [&](auto a, auto lin) {
      assign_state(sb, a, lin);
    };

    assign(a, nab_lin(1));
    assign(b, vs_shared_t(new vs_finite( { 2, 3, 4 })));
    assign(c, vs_shared_t(new vs_finite( { 1, 8, 10, 12 })));
    assign(d, nab_lin(vs_shared_t(new vs_open(UPWARD, 3))));
    assign(e, nab_lin(vs_shared_t(new vs_open(DOWNWARD, 3))));
    assign(f, nab_lin(vs_shared_t(new vs_open(DOWNWARD, 4))));
    assign(g, vs_shared_t(new vs_finite( { -11, -9, -7 })));
    assign(h, value_set::top);
  }

  vsd_state *joined_comp = gc(new vsd_state());
  {
    auto assign = [&](auto a, auto lin) {
      assign_state(joined_comp, a, lin);
    };

    assign(a, nab_lin(1));
    assign(b, vs_shared_t(new vs_finite( { 2, 3, 4, 90 })));
    assign(c, vs_shared_t(new vs_finite( { 1, 8, 10, 12, 13 })));
    assign(d, nab_lin(vs_shared_t(new vs_open(UPWARD, 1))));
    assign(e, nab_lin(value_set::top));
    assign(f, nab_lin(vs_shared_t(new vs_open(DOWNWARD, 4))));
  }

  vsd_state *joined_ab = gc(sa->join(sb, 0));
  vsd_state *joined_ba = gc(sb->join(sa, 0));

  ASSERT_EQ(*joined_ab, *joined_ba);
  ASSERT_EQ(*joined_comp, *joined_ab);
}
