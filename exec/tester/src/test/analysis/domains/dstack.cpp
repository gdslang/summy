/*
 * dstack.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: Julian Kranz
 */

#include <bjutil/binary/elf_provider.h>
#include <summy/analysis/domains/dstack.h>
#include <summy/analysis/domains/memory_state.h>
#include <summy/test/compile.h>
#include <summy/analysis/fixpoint.h>
#include <summy/big_step/dectran.h>
#include <bjutil/gdsl_init.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/gdsl.h>
#include <summy/cfg/node/node_visitor.h>
#include <summy/cfg/bfs_iterator.h>
#include <gtest/gtest.h>
#include <summy/cfg/node/address_node.h>
#include <memory>
#include <iostream>
#include <map>

using namespace analysis;
using namespace std;
using namespace gdsl::rreil;
using namespace cfg;

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

struct _analysis_result {
  dstack *ds_analyzed;
  map<size_t, size_t> addr_node_map;
  elf_provider *elfp;

  ~_analysis_result() {
    delete ds_analyzed;
    delete elfp;
  }
};

static void state_asm(_analysis_result &r, string _asm) {
  SCOPED_TRACE("state_asm()");

  r.elfp = asm_compile_elfp(_asm);

//  binary_provider::entry_t e;
//  tie(ignore, e) = elfp->entry("foo");
//  cout << e.address << " " << e.offset << " " << e.size << endl;

  auto compiled = asm_compile(_asm);

  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  g.set_code(compiled.data(), compiled.size(), 0);

  dectran dt(g, false);
  dt.transduce();
  dt.register_();

  auto &cfg = dt.get_cfg();
  cfg.commit_updates();

  for(auto *node : cfg) {
    bool _break = false;
    node_visitor nv;
    nv._([&](address_node *an) {
      r.addr_node_map[an->get_address()] = an->get_id();
    });
    node->accept(nv);
    if(_break) break;
  }

  r.ds_analyzed = new dstack(&cfg);
  fixpoint fp(r.ds_analyzed);

  fp.iterate();
}

static void query_val(vs_shared_t &r, _analysis_result &ar, string label, string arch_id_name, size_t offset,
    size_t size) {
  SCOPED_TRACE("query_val()");

  auto analy_r = ar.ds_analyzed->result();

  bool found;
  binary_provider::entry_t e;
  tie(found, e) = ar.elfp->entry(label);
  ASSERT_TRUE(found);

  auto addr_it = ar.addr_node_map.find(e.address);
  ASSERT_NE(addr_it, ar.addr_node_map.end());

  ASSERT_GT(analy_r.result.size(), addr_it->second);

  lin_var *lv = new lin_var(new variable(new arch_id(arch_id_name), offset));
  r = analy_r.result[ar.addr_node_map[e.address]]->queryVal(lv, size);
  delete lv;
}

TEST_F(dstack_test, Basics) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "mov $99, %rax\n\
   first: mov $20, %rax\n\
   second: nop\n"));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "first", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(99));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(20));
}

TEST_F(dstack_test, OneFieldReplacesTwoFields) {
  /*
   * query_region + == auf regions
   */

}

TEST_F(dstack_test, TwoFieldsReplaceOneField) {

}
