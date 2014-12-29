/*
 * ismt.cpp
 *
 *  Created on: Dec 29, 2014
 *      Author: Julian Kranz
 */


#include <gtest/gtest.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/gdsl.h>
#include <summy/test/asm_compile.h>
#include <summy/big_step/dectran.h>
#include <summy/big_step/ssa.h>
#include <summy/cfg/cfg.h>
#include <summy/analysis/ismt/ismt.h>
#include <fstream>
#include <iosfwd>
#include <set>
#include <string>

using namespace std;
using namespace analysis;

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

static void targets_for_asm(set<size_t> &targets, string _asm) {
  auto compiled = asm_compile(_asm);

  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  g.set_code(compiled.data(), compiled.size(), 0);

  dectran dt(g, false);
  dt.transduce();
  dt.register_();

  cfg::cfg &cfg = dt.get_cfg();

  cfg.commit_updates();

  ofstream dot_fs;
  dot_fs.open("output.dot", ios::out);
  cfg.dot(dot_fs);
  dot_fs.close();

  ssa ssa(cfg);
  ssa.transduce();

  ismt _ismt(&cfg, ssa.lv_result(), ssa.rd_result());
  auto unres_all = dt.get_unresolved();
  ASSERT_EQ(unres_all.size(), 1);
  size_t unres = *unres_all.begin();

  ismt_edge_ass_t asses_loc = _ismt.analyse(unres);
  ASSERT_EQ(asses_loc.size(), 1);

  targets = asses_loc.begin()->second;
  ASSERT_EQ(targets.size(), 1);
}

TEST_F(ismt_test, SimplePartialRegisterWrites) {
  set<size_t> targets;
  targets_for_asm(targets,
  "mov $999, %rax\n\
  add $42, %rax\n\
  mov $62, %ah\n\
  jmp *%rax\n");

  size_t value = *targets.begin();
  ASSERT_EQ(value, 15889);
}

TEST_F(ismt_test, SimplePartialRegisterWrites2) {
  set<size_t> targets;
  targets_for_asm(targets,
  "mov $999, %rax\n"
  "add $42, %rax\n"
  "mov $77, %bh\n"
  "mov %bh, %ah\n"
  "jmp *%rax\n");

  size_t value = *targets.begin();
  ASSERT_EQ(value, 19729);
}

TEST_F(ismt_test, SimplePartialRegisterWrites3) {
  set<size_t> targets;
  targets_for_asm(targets,
  "movq $342, %rax\n"
  "add $10, %ah\n"
  "jmp *%rax\n");

  size_t value = *targets.begin();
  ASSERT_EQ(value, 2902);
}
