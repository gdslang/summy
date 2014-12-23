/*
 * test.cpp
 *
 *  Created on: Dec 23, 2014
 *      Author: Julian Kranz
 */

#include <summy/test/test.h>
#include <gtest/gtest.h>
#include <stdint.h>

int summy_test_run(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
