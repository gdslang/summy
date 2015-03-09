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

template<typename T>
static void repl(T *(&o), T *n) {
  delete o;
  o = n;
}

TEST_F(vsd_state_test, SimpleAssignments) {
  //automatic queue gc??

  vsd_state *s = new vsd_state(elements_t {});

  auto a = var_temporary();
  repl(s, s->assign(a, nap_lin(1).expr()));

  cout << *s << endl;

  delete a;
  delete s;
}
