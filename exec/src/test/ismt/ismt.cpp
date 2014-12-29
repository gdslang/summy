/*
 * ismt.cpp
 *
 *  Created on: Dec 29, 2014
 *      Author: Julian Kranz
 */


#include <gtest/gtest.h>

class ismt_test: public ::testing::Test {
protected:

  ismt_test() {
  }

  virtual ~ismt_test() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

TEST_F(ismt_test, Blahblah) {
  ASSERT_TRUE(1);
}
