/*
 * dstack.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: Julian Kranz
 */

#include <bjutil/binary/elf_provider.h>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <summy/test/compile.h>
#include <summy/analysis/fixpoint.h>
#include <summy/big_step/dectran.h>
#include <bjutil/gdsl_init.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/gdsl.h>
#include <summy/cfg/node/node_visitor.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/cfg/bfs_iterator.h>
#include <gtest/gtest.h>
#include <summy/cfg/jd_manager.h>
#include <summy/cfg/node/address_node.h>
#include <memory>
#include <iostream>
#include <map>

using namespace analysis;
using namespace analysis::api;
using namespace std;
using namespace gdsl::rreil;
using namespace cfg;
using namespace summy::rreil;

using namespace std;
using namespace summy;

class summary_dstack_test: public ::testing::Test {
protected:

  summary_dstack_test() {
  }

  virtual ~summary_dstack_test() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

struct _analysis_result {
  dectran *dt;

  summary_dstack *ds_analyzed;
  map<size_t, size_t> addr_node_map;
  elf_provider *elfp;

  _analysis_result() {
    dt = NULL;
    ds_analyzed = NULL;
    elfp = NULL;
  }

  ~_analysis_result() {
    delete ds_analyzed;
    delete elfp;
    delete dt;

    ds_analyzed = NULL;
    elfp = NULL;
    dt = NULL;
  }
};

static void state_asm(_analysis_result &r, string _asm, bool gdsl_optimize = false) {
  SCOPED_TRACE("state_asm()");

  r.elfp = asm_compile_elfp(_asm);

//  binary_provider::entry_t e;
//  tie(ignore, e) = elfp->entry("foo");
//  cout << e.address << " " << e.offset << " " << e.size << endl;

  auto compiled = asm_compile(_asm);

  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  binary_provider::entry_t section;
  bool success;
  tie(success, section) = r.elfp->section(".text");
  if(!success)
    throw string("Invalid section .text");

  binary_provider::entry_t function;
  tie(ignore, function) = r.elfp->symbol("main");

//  unsigned char *buffer = (unsigned char*)malloc(section.size);
//  memcpy(buffer, elfp.get_data().data + section.offset, section.size);

  size_t size = (function.offset - section.offset) + function.size + 1000;
  if(size > section.size)
    size = section.size;

  g.set_code(r.elfp->get_data().data + section.offset, size, section.address);
  if(g.seek(function.address)) {
    throw string("Unable to seek to given function_name");
  }

  r.dt = new dectran(g, gdsl_optimize);
  r.dt->transduce();
  r.dt->register_();

  auto &cfg = r.dt->get_cfg();
  cfg.commit_updates();

  r.ds_analyzed = new summary_dstack(&cfg);
  jd_manager jd_man(&cfg);
  fixpoint fp(r.ds_analyzed, jd_man);
  fp.iterate();


  for(auto *node : cfg) {
    node_visitor nv;
    nv._([&](address_node *an) {
      an->dot(cout);
      cout << endl;
      r.addr_node_map[an->get_address()] = an->get_id();
    });
    node->accept(nv);
  }

  cfg.dot(cout);
  cout << endl;
}

static void query_val(vs_shared_t &r, _analysis_result &ar, string label, string arch_id_name, size_t offset,
    size_t size) {
  SCOPED_TRACE("query_val()");

  auto analy_r = ar.ds_analyzed->result();

  bool found;
  binary_provider::entry_t e;
  tie(found, e) = ar.elfp->symbol(label);
  ASSERT_TRUE(found);

  auto addr_it = ar.addr_node_map.find(e.address);
  ASSERT_NE(addr_it, ar.addr_node_map.end());

  ASSERT_GT(analy_r.result.size(), addr_it->second);

  lin_var *lv = new lin_var(new variable(new arch_id(arch_id_name), offset));
  cout << e.address << endl;
  cout << ar.addr_node_map[e.address] << endl;
//  cout << *analy_r.result[ar.addr_node_map[e.address]]->get_mstate() << endl;
  r = analy_r.result[ar.addr_node_map[e.address]]->get_mstate()->queryVal(lv, size);
  delete lv;
}

static void query_eq(vs_shared_t &r, _analysis_result &ar, string label, string arch_id_first, string arch_id_second) {
//  SCOPED_TRACE("query_eq()");
//
//  auto analy_r = ar.ds_analyzed->result();
//
//  bool found;
//  binary_provider::entry_t e;
//  tie(found, e) = ar.elfp->symbol(label);
//  ASSERT_TRUE(found);
//
//  auto addr_it = ar.addr_node_map.find(e.address);
//  ASSERT_NE(addr_it, ar.addr_node_map.end());
//
//  ASSERT_GT(analy_r.result.size(), addr_it->second);
//
//  lin_var *lv_first = new lin_var(new variable(new arch_id(arch_id_first), 0));
//  lin_var *lv_second = new lin_var(new variable(new arch_id(arch_id_second), 0));
//  expr *ec = new expr_sexpr(new sexpr_cmp(64, new expr_cmp(CMP_EQ, lv_first, lv_second)));
//  r = analy_r.result[ar.addr_node_map[e.address]]->queryVal(ec, 64);
//  delete ec;
}

static void equal_structure(region_t const& a, region_t const& b) {
//  SCOPED_TRACE("equal_structure()");
//  ASSERT_EQ(a.size(), b.size());
//  for(auto &a_it : a) {
//    auto b_it = b.find(a_it.first);
//    ASSERT_NE(b_it, b.end());
//    ASSERT_EQ(a_it.second.size, b_it->second.size);
//  }
}

static void equal_structure(region_t const& cmp, _analysis_result &ar, string label, string arch_id_name) {
//  SCOPED_TRACE("equal_structure()");
//
//  auto analy_r = ar.ds_analyzed->result();
//
//  bool found;
//  binary_provider::entry_t e;
//  tie(found, e) = ar.elfp->symbol(label);
//  ASSERT_TRUE(found);
//
//  auto addr_it = ar.addr_node_map.find(e.address);
//  ASSERT_NE(addr_it, ar.addr_node_map.end());
//
//  ASSERT_GT(analy_r.result.size(), addr_it->second);
//
//  id_shared_t id = shared_ptr<gdsl::rreil::id>(new arch_id(arch_id_name));
//  region_t const& rr = analy_r.result[ar.addr_node_map[e.address]]->query_region(id);
//
//  equal_structure(cmp, rr);
}

static void query_als(ptr_set_t &aliases, _analysis_result &ar, string label, string arch_id_name) {
//  SCOPED_TRACE("query_als()");
//
//  auto analy_r = ar.ds_analyzed->result();
//
//  bool found;
//  binary_provider::entry_t e;
//  tie(found, e) = ar.elfp->symbol(label);
//  ASSERT_TRUE(found);
//
//  auto addr_it = ar.addr_node_map.find(e.address);
//  ASSERT_NE(addr_it, ar.addr_node_map.end());
//
//  ASSERT_GT(analy_r.result.size(), addr_it->second);
//
//  address *a = new address(64, new lin_var(new variable(new arch_id(arch_id_name), 0)));
//  aliases = analy_r.result[ar.addr_node_map[e.address]]->queryAls(a);
//  delete a;
}


TEST_F(summary_dstack_test, Call) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
"g:\n\
mov %r13, %r14\n\
ret\n\
f:\n\
mov (%r11), %r12\n\
mov %r12, %rax\n\
ret\n\
\n\
main:\n\
movq $42, (%rdx)\n\
mov (%rbx), %rcx\n\
mov %rdx, (%rcx)\n\
mov %rbx, %r11\n\
call f\n\
movq (%rax), %r13\n\
movq (%r13), %r13\n\
end: ret", false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R13", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(42));
}
