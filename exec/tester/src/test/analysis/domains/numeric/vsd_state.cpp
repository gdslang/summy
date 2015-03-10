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

  vsd_state *s = gc(new vsd_state(elements_t {}));

  auto a = rreil_builder::temporary();
  auto b = rreil_builder::temporary();
  auto c = rreil_builder::temporary();
  s = gc(s->assign(n.var(a), n.expr(nab_lin(1))));
  s = gc(s->assign(n.var(b), n.expr(nab_lin(vs_shared_t(new vs_finite({2, 3, 4}))))));
  s = gc(s->assign(n.var(c), n.expr((a + (2 * b + 3)))));
  auto d = rreil_builder::temporary();
  s = gc(s->assign(n.var(d), n.expr(nab_lin(vs_shared_t(new vs_open(UPWARD, 3))))));
  auto e = rreil_builder::temporary();
  s = gc(s->assign(n.var(e), n.expr((a + (- c)) + 5 * d)));
  auto f = rreil_builder::temporary();
  s = gc(s->assign(n.var(f), n.expr(a + e)));

  cout << *s << endl;
}
