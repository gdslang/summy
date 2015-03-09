/*
 * vsd_state.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: Julian Kranz
 */

#include <gtest/gtest.h>
#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/test/analysis/domains/numeric_api_builder.h>

using namespace analysis::api;
using namespace analysis::value_sets;
using namespace analysis;

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

TEST_F(vsd_state_test, SimpleAssignments) {
  vsd_state *s = new vsd_state(elements_t {});
}
