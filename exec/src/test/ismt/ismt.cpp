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
  size_t asses_count = asses_loc.size();
  ASSERT_LE(asses_count, 1);

  if(asses_count == 1)
    targets = asses_loc.begin()->second;
}

TEST_F(ismt_test, SimplePartialRegisterWrites) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $999, %rax\n\
  add $42, %rax\n\
  mov $62, %ah\n\
  jmp *%rax\n"));

  ASSERT_EQ(targets.size(), 1);
  size_t value = *targets.begin();
  ASSERT_EQ(value, 15889);
}

TEST_F(ismt_test, SimplePartialRegisterWritesUndefined) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $999, %rax\n\
  add $42, %rax\n\
  mov %cl, %ah\n\
  jmp *%rax\n"));

  ASSERT_EQ(targets.size(), 0);
}

TEST_F(ismt_test, SimplePartialRegisterWrites2) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $999, %rax\n"
  "add $42, %rax\n"
  "mov $77, %bh\n"
  "mov %bh, %ah\n"
  "jmp *%rax\n"));

  ASSERT_EQ(targets.size(), 1);
  size_t value = *targets.begin();
  ASSERT_EQ(value, 19729);
}

TEST_F(ismt_test, SimplePartialRegisterWrites3) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "movq $342, %rax\n"
  "add $10, %ah\n"
  "jmp *%rax\n"));

  ASSERT_EQ(targets.size(), 1);
  size_t value = *targets.begin();
  ASSERT_EQ(value, 2902);
}

TEST_F(ismt_test, PartialMemory) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "movq $38813467, %rbx\n"
  "movw $999, (%rax)\n"
  "movb (%rax), %bh\n"
  "jmp *%rbx\n"));

  ASSERT_EQ(targets.size(), 1);
  size_t value = *targets.begin();
  ASSERT_EQ(value, 38856475);
}

TEST_F(ismt_test, PartialMemoryUndefined) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "movw $999, (%rax)\n"
  "movb (%rax), %bh\n"
  "jmp *%rbx\n"));

  ASSERT_EQ(targets.size(), 0);
}

TEST_F(ismt_test, AddressCalculations) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $0x0000776655443322, %rax\n"
  "mov $147, %cl\n"
  "and $0xfecda11, %rax\n"
  "mov $0x3f, %bl\n"
  "movsx %bl, %rbx\n"
  "xor %cl, %ah\n"
  "lea (%rax, %rbx, 2), %rax\n"
  "jmp *%rax\n"));

  ASSERT_EQ(targets.size(), 1);
  size_t value = *targets.begin();
  ASSERT_EQ(value, 0x544817e);
}

TEST_F(ismt_test, Shift) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $0x0000776655443322, %rax\n"
  "shl $27, %rax\n"
  "jmp *%rax\n"));

  ASSERT_EQ(targets.size(), 2);
  auto targets_it = targets.begin();
  ASSERT_EQ(*targets_it, 131281400902434);
  targets_it++;
  ASSERT_EQ(*targets_it, 3650767389219356672);
}

TEST_F(ismt_test, ShiftUndefined) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $0x0000776655443322, %rax\n"
  "shl %cl, %rax\n"
  "jmp *%rax\n"));

  ASSERT_EQ(targets.size(), 1);
  size_t value = *targets.begin();
  ASSERT_EQ(value, 0x0000776655443322);
}

TEST_F(ismt_test, IfThenElseIndependent) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $999, %rbx\n"
  "cmp $99, %rax\n"
  "jne else\n"
  "mov $999, %rcx\n"
  "jmp after\n"
  "else:\n"
  "mov $777, %rcx\n"
  "after:\n"
  "jmp *%rbx\n"));

  ASSERT_EQ(targets.size(), 1);
  size_t value = *targets.begin();
  ASSERT_EQ(value, 999);
}

TEST_F(ismt_test, IfThenElseFullyDefined1) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $207, %rbx\n"
  "mov $99, %rax\n"
  "cmp $99, %rax\n"
  "jne else\n"
  "add $99, %rbx\n"
  "mov $333, %rbx\n"
  "jmp after\n"
  "else:\n"
  "sub $77, %rbx\n"
  "after:\n"
  "jmp *%rbx\n"));

  ASSERT_EQ(targets.size(), 2);
  auto targets_it = targets.begin();
  ASSERT_EQ(*targets_it, 130);
  targets_it++;
  ASSERT_EQ(*targets_it, 333);
}

TEST_F(ismt_test, IfThenElseFullyDefined2) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $207, %rbx\n"
  "mov $99, %rax\n"
  "cmp $99, %rax\n"
  "jne else\n"
  "add %rax, %rbx\n"
  "jmp after\n"
  "else:\n"
  "sub $77, %rbx\n"
  "after:\n"
  "jmp *%rbx\n"));

  ASSERT_EQ(targets.size(), 2);
  auto targets_it = targets.begin();
  ASSERT_EQ(*targets_it, 130);
  targets_it++;
  ASSERT_EQ(*targets_it, 306);
}

TEST_F(ismt_test, IfThenElseOneBranchDefined1) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $207, %rbx\n"
  "mov $99, %rax\n"
  "cmp $99, %rax\n"
  "jne else\n"
  "add %rcx, %rbx\n"
  "jmp after\n"
  "else:\n"
  "sub $77, %rbx\n"
  "after:\n"
  "jmp *%rbx\n"));

  ASSERT_EQ(targets.size(), 1);
  size_t value = *targets.begin();
  ASSERT_EQ(value, 130);
}

TEST_F(ismt_test, IfThenElseOneBranchDefined2) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $99, %rax\n"
  "cmp $99, %rax\n"
  "jne else\n"
  "mov $1000, %rbx\n"
  "jmp after\n"
  "else:\n"
  "after:\n"
  "jmp *%rbx\n"));

  ASSERT_EQ(targets.size(), 1);
  size_t value = *targets.begin();
  ASSERT_EQ(value, 1000);
}

TEST_F(ismt_test, IfThenElseFullyUndefined) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $207, %rbx\n"
  "mov $99, %rax\n"
  "cmp $99, %rax\n"
  "jne else\n"
  "add %rcx, %rbx\n"
  "jmp after\n"
  "else:\n"
  "sub %rdx, %rbx\n"
  "sub $77, %rbx\n"
  "after:\n"
  "jmp *%rbx\n"));

  ASSERT_EQ(targets.size(), 0);
}

TEST_F(ismt_test, IfThenElseOneBranchDefinedDoubleVar) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $207, %rbx\n"
  "mov $99, %rax\n"
  "cmp $99, %rax\n"
  "jne else\n"
  "add $99, %rbx\n"
  "jmp after\n"
  "else:\n"
  "sub $77, %rbx\n"
  "mov $57, %rcx\n"
  "after:\n"
  "add %rcx, %rbx\n"
  "jmp *%rbx\n"));

  ASSERT_EQ(targets.size(), 1);
  size_t value = *targets.begin();
  ASSERT_EQ(value, 187);
}

TEST_F(ismt_test, IfThenElseFullyDefinedDoubleVar) {
  set<size_t> targets;
  ASSERT_NO_FATAL_FAILURE(targets_for_asm(targets,
  "mov $207, %rbx\n"
  "mov $99, %rcx\n"
  "cmp $99, %rax\n"
  "jne else\n"
  "add $8, %rbx\n"
  "jmp after\n"
  "else:\n"
  "sub $77, %rbx\n"
  "mov $57, %rcx\n"
  "after:\n"
  "add %rcx, %rbx\n"
  "jmp *%rbx\n"));

  ASSERT_EQ(targets.size(), 2);
  auto targets_it = targets.begin();
  ASSERT_EQ(*targets_it, 187);
  targets_it++;
  ASSERT_EQ(*targets_it, 314);
}

