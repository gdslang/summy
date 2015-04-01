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
#include <iostream>

using namespace analysis;
using namespace std;
using namespace gdsl::rreil;

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

static void state_asm(dstack *&ds_analyzed, string _asm) {
  auto compiled = asm_compile(_asm);

  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  g.set_code(compiled.data(), compiled.size(), 0);

  dectran dt(g, false);
  dt.transduce();
  dt.register_();

  auto &cfg = dt.get_cfg();
  cfg.commit_updates();

  ds_analyzed = new dstack(&cfg);
  fixpoint fp(ds_analyzed);

  fp.iterate();
}

TEST_F(dstack_test, OneFieldReplacesTwoFields) {
  dstack *ds_analyzed;
//  ASSERT_NO_FATAL_FAILURE(state_asm(ds_analyzed,
//  "mov $99, %rax\n\
//   mov $20, %eax\n"));
    ASSERT_NO_FATAL_FAILURE(state_asm(ds_analyzed,
    "mov $99, %rax\n"));

  cout << "FOOOOOOOOOOOO" << endl;

  auto result = ds_analyzed->result();
//  cout << *result.result[4]->queryVal(new lin_var(new variable(new arch_id("A"), 0))) << endl;

  /*
   * Todo: Zunächst ein Feld, dann zwei
   */

  delete ds_analyzed;
}

TEST_F(dstack_test, TwoFieldsReplaceOneField) {

}
