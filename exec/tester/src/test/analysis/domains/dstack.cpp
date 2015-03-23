/*
 * dstack.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/dstack.h>
#include <summy/analysis/domains/memory_state.h>
#include <summy/test/compile.h>
#include <summy/analysis/fixpoint.h>
#include <summy/big_step/dectran.h>
#include <bjutil/gdsl_init.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/gdsl.h>
#include <gtest/gtest.h>
#include <memory>

using namespace analysis;
using namespace std;

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

static void final_state_asm(shared_ptr<memory_state> &final_state, string _asm) {
  auto compiled = asm_compile(_asm);

  for(auto byte : compiled)
    printf("0x%x, ", byte);

  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  g.set_code(compiled.data(), compiled.size(), 0);

  dectran dt(g, false);
  dt.transduce();
  dt.register_();

  auto &cfg = dt.get_cfg();
  cfg.commit_updates();

//  dstack ds(&cfg);
//  fixpoint fp(&ds);
//
//  fp.iterate();
//
//  dstack_result dr = ds.result();
//  final_state = *(--dr.result.end());
}

TEST_F(dstack_test, OneFieldReplacesTwoFields) {
  shared_ptr<memory_state> final_state;
  ASSERT_NO_FATAL_FAILURE(final_state_asm(final_state,
  "mov $99, %rax\n\
   mov $20, %eax\n"));

}

TEST_F(dstack_test, TwoFieldsReplaceOneField) {

}
