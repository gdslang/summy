/*
 * value_set.cpp
 *
 *  Created on: Mar 11, 2015
 *      Author: Julian Kranz
 */

#include <gtest/gtest.h>
#include <summy/value_set/value_set.h>
#include <memory>

using namespace std;
using namespace summy;

class value_set_test: public ::testing::Test {
protected:

  value_set_test() {
  }

  virtual ~value_set_test() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

//vs_shared_t s1 = make_shared<vs_finite>(vs_finite::elements_t {4, 2, 3});
//vs_shared_t s9 = make_shared<vs_finite>(vs_finite::elements_t {0, 4, 2, 3});
//  vs_shared_t s2 = make_shared<vs_finite>(vs_finite::elements_t {1, 9});
//  vs_shared_t s10 = make_shared<vs_finite>(vs_finite::elements_t {-2, 3});
//  vs_shared_t s11 = -(*s1);
//
//  vs_shared_t s3 = value_set::join(s1, s2);
//  vs_shared_t s4 = value_set::widen(s1, s2);
//
//  vs_shared_t s5 = *s1 * s2;
//vs_shared_t s6 = make_shared<vs_open>(DOWNWARD, 3);
//vs_shared_t s8 = make_shared<vs_open>(DOWNWARD, -9);
//
//  vs_shared_t s7 = *s1 * s6;
//  cout << *s1 << "*" << *s6 << " = " << *s7 << endl;
//  cout << *s1 << "/" << *s6 << " = " << *(*s1 / s6) << endl;
//  cout << *s1 << "/" << *s8 << " = " << *(*s1 / s8) << endl;
//  cout << *s9 << "/" << *s8 << " = " << *(*s9 / s8) << endl;
//  cout << *s9 << "+" << *s2 << " = " << *(*s9 + s2) << endl;
//  cout << *s1 << "*" << *s10 << " = " << *(*s1 * s10) << endl;
//  cout << *s1 << "*" << *s8 << " = " << *(*s1 * s8) << endl;
//  cout << *s11 << "*" << *s8 << " = " << *(*s11 * s8) << endl;

//  cout << *s1 << "/" << *s6 << " = " << *(*s1 / s6) << endl;
//  cout << *s1 << "/" << *s8 << " = " << *(*s1 / s8) << endl;
//  cout << *s9 << "/" << *s8 << " = " << *(*s9 / s8) << endl;

TEST_F(value_set_test, Addition) {
  vs_shared_t s1 = make_shared<vs_finite>(vs_finite::elements_t { 4, 2, 3 });
  vs_shared_t s9 = make_shared<vs_finite>(vs_finite::elements_t { 0, 4, 2, 3 });
  vs_shared_t s2 = make_shared<vs_finite>(vs_finite::elements_t { 1, 9 });
  vs_shared_t s6 = make_shared<vs_open>(DOWNWARD, 3);
  vs_shared_t s8 = make_shared<vs_open>(UPWARD, -9);
  vs_shared_t s11 = -(*s1);

  vs_shared_t s9_plus_s2 = make_shared<vs_finite>(vs_finite::elements_t { 1, 3, 4, 5, 9, 11, 12, 13 });
  vs_shared_t s1_plus_s6 = make_shared<vs_open>(DOWNWARD, 7);
  vs_shared_t s11_plus_s8 = make_shared<vs_open>(UPWARD, -13);

  ASSERT_EQ(*(*s9 + s2), s9_plus_s2);
  ASSERT_EQ(*(*s1 + s6), s1_plus_s6);
  ASSERT_EQ(*(*s11 + s8), s11_plus_s8);
}

TEST_F(value_set_test, Multiplication) {
  vs_shared_t s1 = make_shared<vs_finite>(vs_finite::elements_t { 4, 2, 3 });
  vs_shared_t s2 = make_shared<vs_finite>(vs_finite::elements_t { 1, 9 });
  vs_shared_t s10 = make_shared<vs_finite>(vs_finite::elements_t { -2, 3 });
  vs_shared_t s11 = -(*s1);

  vs_shared_t s5 = *s1 * s2;
  vs_shared_t s6 = make_shared<vs_open>(DOWNWARD, 3);
  vs_shared_t s8 = make_shared<vs_open>(DOWNWARD, -9);

  vs_shared_t s1_times_s6 = *s1 * s6;
  vs_shared_t s1_times_s10 = make_shared<vs_finite>(vs_finite::elements_t {-8, -6, -4, 6, 9, 12});
  vs_shared_t s1_times_s8 = make_shared<vs_open>(DOWNWARD, -18);
  vs_shared_t s11_times_s8 = make_shared<vs_open>(UPWARD, 18);

  ASSERT_EQ(*(*s1 * s6), s1_times_s6);
  ASSERT_EQ(*(*s1 * s10), s1_times_s10);
  ASSERT_EQ(*(*s1 * s8), s1_times_s8);
  ASSERT_EQ(*(*s11 * s8), s11_times_s8);

}

TEST_F(value_set_test, Division) {
  vs_shared_t s1 = make_shared<vs_finite>(vs_finite::elements_t { 4, 2, 3 });
  vs_shared_t s9 = make_shared<vs_finite>(vs_finite::elements_t { 0, 4, 2, 3 });
  vs_shared_t s6 = make_shared<vs_open>(DOWNWARD, 3);
  vs_shared_t s8 = make_shared<vs_open>(DOWNWARD, -9);

  vs_shared_t sa = make_shared<vs_finite>(vs_finite::elements_t { 0, 4, 2, 3, 5, 9, 20 });
  vs_shared_t sb = make_shared<vs_finite>(vs_finite::elements_t { 2, 3 });

  vs_shared_t s1_div_s6 = make_shared<vs_finite>(vs_finite::elements_t { -4, -3, -2, -1, 0, 1, 2, 3, 4 });
  vs_shared_t s1_div_s8 = make_shared<vs_finite>(vs_finite::elements_t { -4, -3, -2, -1, 0 });
  vs_shared_t s9_div_s8 = make_shared<vs_finite>(vs_finite::elements_t { -4, -3, -2, -1, 0 });
  vs_shared_t sa_div_sb = make_shared<vs_finite>(vs_finite::elements_t { 0, 2, 1, 3, 10 });

  ASSERT_EQ(*(*s1 / s6), s1_div_s6);
  ASSERT_EQ(*(*s1 / s8), s1_div_s8);
  ASSERT_EQ(*(*s9 / s8), s9_div_s8);
  ASSERT_EQ(*(*sa / sb), sa_div_sb);
}
