/*
 * dstack.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/dstack.h>
#include <gtest/gtest.h>
#include <memory>


/*
 * Tests für
 *
 * mov $99, %rax
 * mov $20, %eax
 *
 * und
 *
 * mov $20, %eax
 * mov $99, %rax
 *
 * Überlappende Felder müssen hier entsprechend ausgetauscht werden.
 *
 * Test für
 *
 * mov $20, %eax
 * add $10, %rax
 */

using namespace std;
using namespace summy;

class dstack_test: public ::testing::Test {
protected:

  dstack_test() {
  }

  virtual ~dstack_test() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

TEST_F(dstack_test, OneFieldReplacesTwoFields) {

}

TEST_F(dstack_test, TwoFieldsReplaceOneField) {

}
