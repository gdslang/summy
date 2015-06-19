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
#include <set>
#include <algorithm>

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

  shared_ptr<static_memory> se = make_shared<static_elf>(r.elfp);
  r.ds_analyzed = new summary_dstack(&cfg, se);
  jd_manager jd_man(&cfg);
  fixpoint fp(r.ds_analyzed, jd_man);
  fp.iterate();


  for(auto *node : cfg) {
    node_visitor nv;
    nv._([&](address_node *an) {
      r.addr_node_map[an->get_address()] = an->get_id();
    });
    node->accept(nv);
  }
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
//  cout << *analy_r.result[ar.addr_node_map[e.address]]->get_mstate() << endl;
  r = analy_r.result[ar.addr_node_map[e.address]]->get_mstate()->queryVal(lv, size);
  delete lv;
}

//static void query_eq(vs_shared_t &r, _analysis_result &ar, string label, string arch_id_first, string arch_id_second) {
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
//}
//
//static void equal_structure(region_t const& a, region_t const& b) {
//  SCOPED_TRACE("equal_structure()");
//  ASSERT_EQ(a.size(), b.size());
//  for(auto &a_it : a) {
//    auto b_it = b.find(a_it.first);
//    ASSERT_NE(b_it, b.end());
//    ASSERT_EQ(a_it.second.size, b_it->second.size);
//  }
//}

static void equal_structure(region_t const &a, region_t const &b) {
  SCOPED_TRACE("equal_structure()");
  ASSERT_EQ(a.size(), b.size());
  for(auto &a_it : a) {
    auto b_it = b.find(a_it.first);
    ASSERT_NE(b_it, b.end());
    ASSERT_EQ(a_it.second.size, b_it->second.size);
  }
}

static void equal_structure(region_t const& cmp, _analysis_result &ar, string label, string arch_id_name) {
  SCOPED_TRACE("equal_structure()");

  auto analy_r = ar.ds_analyzed->result();

  bool found;
  binary_provider::entry_t e;
  tie(found, e) = ar.elfp->symbol(label);
  ASSERT_TRUE(found);

  auto addr_it = ar.addr_node_map.find(e.address);
  ASSERT_NE(addr_it, ar.addr_node_map.end());

  ASSERT_GT(analy_r.result.size(), addr_it->second);

  id_shared_t id = shared_ptr<gdsl::rreil::id>(new arch_id(arch_id_name));
  region_t const& rr = analy_r.result[ar.addr_node_map[e.address]]->get_mstate()->query_region_output(id);

  equal_structure(cmp, rr);
}

static void mstate_from_label(summary_memory_state **mstate, _analysis_result &ar, string label) {
  SCOPED_TRACE("mstate_from_label()");

  auto analy_r = ar.ds_analyzed->result();

  bool found;
  binary_provider::entry_t e;
  tie(found, e) = ar.elfp->symbol(label);
  ASSERT_TRUE(found);

  auto addr_it = ar.addr_node_map.find(e.address);
  ASSERT_NE(addr_it, ar.addr_node_map.end());

  ASSERT_GT(analy_r.result.size(), addr_it->second);

  *mstate = analy_r.result[ar.addr_node_map[e.address]]->get_mstate();
}

static void query_deref_als(ptr_set_t &aliases, _analysis_result &ar, summary_memory_state *mstate, ptr _ptr) {
  SCOPED_TRACE("query_deref_als()");

  int64_t offset;
  value_set_visitor vsv;
  vsv._([&](vs_finite *vsf) {
    if(!vsf->is_singleton())
      throw string("query_deref_als(): need singleton alias set :-/");
    offset = *vsf->get_elements().begin();
  });
  _ptr.offset->accept(vsv);

  num_var *ptr_var = new num_var(_ptr.id);

  num_linear *derefed = mstate->dereference(ptr_var, 8*offset, 64);
  delete ptr_var;

//  if(*derefed == *value_set::top) {
//    FAIL() << "mstate->dereference yielded top :-(";
//  }

  num_var *derefed_var;
  num_visitor nv;
  nv._([&](num_linear_term *nt) {
    ASSERT_EQ(nt->get_scale(), 1);
    num_visitor nv_inner;
    nv_inner._([&](num_linear_vs *vs) {
      ASSERT_EQ(*vs->get_value_set(), vs_finite::zero);
    });
    nt->get_next()->accept(nv_inner);
    derefed_var = nt->get_var();
  });
  derefed->accept(nv);

  aliases = mstate->queryAls(derefed_var);
  delete derefed;
}

static void query_deref_als(ptr_set_t &aliases, _analysis_result &ar, summary_memory_state *mstate, string arch_id_name) {
  SCOPED_TRACE("query_deref_als()");

  address *a = new address(64, new lin_var(new variable(new arch_id(arch_id_name), 0)));
  ptr_set_t addresses = mstate->queryAls(a);
  ptr const &p = *addresses.begin();

  query_deref_als(aliases, ar, mstate, p);

  delete a;
}

static void query_deref_als(ptr_set_t &aliases, _analysis_result &ar, string label, string arch_id_name) {
  SCOPED_TRACE("query_deref_als()");
  summary_memory_state *mstate;
  mstate_from_label(&mstate, ar, label);
  query_deref_als(aliases, ar, mstate, arch_id_name);
}

static void query_deref_als(ptr_set_t &aliases, _analysis_result &ar, string label, ptr _ptr) {
  SCOPED_TRACE("query_deref_als()");
  summary_memory_state *mstate;
  mstate_from_label(&mstate, ar, label);
  query_deref_als(aliases, ar, mstate, _ptr);
}

static void query_als(ptr_set_t &aliases, _analysis_result &ar, string label, string arch_id_name) {
  SCOPED_TRACE("query_als()");

  auto analy_r = ar.ds_analyzed->result();

  bool found;
  binary_provider::entry_t e;
  tie(found, e) = ar.elfp->symbol(label);
  ASSERT_TRUE(found);

  auto addr_it = ar.addr_node_map.find(e.address);
  ASSERT_NE(addr_it, ar.addr_node_map.end());

  ASSERT_GT(analy_r.result.size(), addr_it->second);

//  cout << *analy_r.result[ar.addr_node_map[e.address]]->get_mstate() << endl;

  address *a = new address(64, new lin_var(new variable(new arch_id(arch_id_name), 0)));
  aliases = analy_r.result[ar.addr_node_map[e.address]]->get_mstate()->queryAls(a);
  delete a;
}

TEST_F(summary_dstack_test, FromDstack_Basics) {
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

TEST_F(summary_dstack_test, FromDstack_OneFieldReplacesTwoFields) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "mov $99, %eax\n\
   first: mov $20, %rax\n\
   second: nop\n"));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "first", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(99));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(20));

  {
    region_t cmp;
    cmp.insert(make_pair(0, field { 32, numeric_id::generate() }));
    cmp.insert(make_pair(32, field { 32, numeric_id::generate() }));
    equal_structure(cmp, ar, "first", "A");
  }
  {
    region_t cmp;
    cmp.insert(make_pair(0, field { 64, numeric_id::generate() }));
    equal_structure(cmp, ar, "second", "A");
  }
}

TEST_F(summary_dstack_test, FieldSplitSameOffset) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "mov $20, %rax\n\
   first: mov $99, %eax\n\
   second: nop\n"));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "first", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(20));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(99));

  {
    region_t cmp;
    cmp.insert(make_pair(0, field { 64, numeric_id::generate() }));
    equal_structure(cmp, ar, "first", "A");
  }
  {
    region_t cmp;
    cmp.insert(make_pair(0, field { 32, numeric_id::generate() }));
    cmp.insert(make_pair(32, field { 32, numeric_id::generate() }));
    equal_structure(cmp, ar, "second", "A");
  }
}

TEST_F(summary_dstack_test, FieldSplitSameOffset2) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "mov $20, %rax\n\
   first: mov $99, %al\n\
   second: nop\n"));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "first", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(20));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 64));
  ASSERT_EQ(*r, value_set::top);
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 8));
  ASSERT_EQ(*r, vs_finite::single(99));

  {
    region_t cmp;
    cmp.insert(make_pair(0, field { 64, numeric_id::generate() }));
    equal_structure(cmp, ar, "first", "A");
  }
  {
    region_t cmp;
    cmp.insert(make_pair(0, field { 8, numeric_id::generate() }));
    cmp.insert(make_pair(8, field { 56, numeric_id::generate() }));
    equal_structure(cmp, ar, "second", "A");
  }
}

TEST_F(summary_dstack_test, FieldSplitMiddle) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "mov $20, %rax\n\
   first: mov $99, %ah\n\
   second: nop\n"));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "first", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(20));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 64));
  ASSERT_EQ(*r, value_set::top);
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 8));
  ASSERT_EQ(*r, vs_finite::top);
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 16));
  ASSERT_EQ(*r, vs_finite::top);
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 8, 8));
  ASSERT_EQ(*r, vs_finite::single(99));

  {
    region_t cmp;
    cmp.insert(make_pair(0, field { 64, numeric_id::generate() }));
    equal_structure(cmp, ar, "first", "A");
  }
  {
    region_t cmp;
    cmp.insert(make_pair(0, field { 8, numeric_id::generate() }));
    cmp.insert(make_pair(8, field { 8, numeric_id::generate() }));
    cmp.insert(make_pair(16, field { 48, numeric_id::generate() }));
    equal_structure(cmp, ar, "second", "A");
  }
}

TEST_F(summary_dstack_test, OverlappingFieldMiddle) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "mov $99, %ah\n\
   first: mov $20, %rax\n\
   second: nop\n"));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "first", "A", 8, 8));
  ASSERT_EQ(*r, vs_finite::single(99));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(20));

  {
    region_t cmp;
    cmp.insert(make_pair(8, field { 8, numeric_id::generate() }));
    equal_structure(cmp, ar, "first", "A");
  }
  {
    region_t cmp;
    cmp.insert(make_pair(0, field { 64, numeric_id::generate() }));
    equal_structure(cmp, ar, "second", "A");
  }
}

TEST_F(summary_dstack_test, Call) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
"f:\n\
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
after_call: movq (%rax), %r13\n\
movq (%r13), %r13\n\
end: ret", false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R13", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(42));

  ptr_set_t aliases_ac_a_deref;
  ASSERT_NO_FATAL_FAILURE(query_deref_als(aliases_ac_a_deref, ar, "after_call", "A"));
  ptr_set_t aliases_ac_d;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_ac_d, ar, "after_call", "D"));
  ASSERT_EQ(aliases_ac_a_deref, aliases_ac_d);
}

TEST_F(summary_dstack_test, CallOffsets) {
  //test21.as using aliases
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
"g:\n\
mov %r13, %r14\n\
ret\n\
\n\
f:\n\
//add $1, %r12\n\
add $1, %r11\n\
mov %r11, %r12\n\
add $8, %r12\n\
//add $8, %r11\n\
ret\n\
\n\
main:\n\
add $2, %r11\n\
before_call:\n\
call f\n\
end: ret", false));

  ptr_set_t aliases_before_call_r11;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_before_call_r11, ar, "before_call", "R11"));
  ASSERT_EQ(aliases_before_call_r11.size(), 1);
  ptr alias_before_call_r11 = *aliases_before_call_r11.begin();

  ptr_set_t aliases_r11;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_r11, ar, "end", "R11"));
  ASSERT_EQ(aliases_r11.size(), 1);
  ASSERT_EQ(aliases_r11, ptr_set_t({ptr(alias_before_call_r11.id, vs_finite::single(3))}));

  ptr_set_t aliases_r12;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_r11, ar, "end", "R12"));
  ASSERT_EQ(aliases_r11.size(), 1);
  ASSERT_EQ(*aliases_r11.begin()->offset, vs_finite::single(11));
}

TEST_F(summary_dstack_test, 2Calls) {
  //test20.as using aliases
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
"f:\n\
mov %r11, %r12\n\
ret\n\
\n\
main:\n\
mov %r14, %r14\n\
mov %r15, %r15\n\
start:\n\
mov %r15, %r11\n\
call f\n\
mov %r12, %rax\n\
mov %r14, %r11\n\
call f\n\
mov %r12, %rbx\n\
end: ret", false));

  ptr_set_t aliases_start_r14;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_start_r14, ar, "start", "R14"));
  ASSERT_EQ(aliases_start_r14.size(), 1);

  ptr_set_t aliases_start_r15;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_start_r15, ar, "start", "R15"));
  ASSERT_EQ(aliases_start_r15.size(), 1);

  ptr_set_t aliases_end_r14;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_end_r14, ar, "end", "R14"));
  ASSERT_EQ(aliases_end_r14, aliases_start_r14);

  ptr_set_t aliases_end_r15;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_end_r15, ar, "end", "R15"));
  ASSERT_EQ(aliases_end_r15, aliases_start_r15);

  ptr_set_t aliases_end_rax;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_end_rax, ar, "end", "A"));
  ASSERT_EQ(aliases_end_rax, aliases_start_r15);

  ptr_set_t aliases_end_rbx;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_end_rbx, ar, "end", "B"));
  ASSERT_EQ(aliases_end_rbx, aliases_start_r14);

}

TEST_F(summary_dstack_test, Call_2AliasesCallee_1AliasesCaller) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
"g:\n\
mov %r13, %r14\n\
ret\n\
\n\
f:\n\
//mov %rcx, %rcx\n\
//mov %rdx, %rdx\n\
je else\n\
mov %rcx, %r11\n\
jmp ite_end\n\
else:\n\
mov %rdx, %r11\n\
ite_end:\n\
mov %r11, (%r12)\n\
ret\n\
\n\
main:\n\
movq $41000, (%rcx)\n\
mov %rcx, %rdx\n\
call f\n\
after_call: mov (%r12), %r12\n\
mov (%r12), %r13\n\
end: ret", false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R13", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(41000));

  ptr_set_t aliases_ac_c;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_ac_c, ar, "after_call", "C"));
  ptr_set_t aliases_ac_d;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_ac_d, ar, "after_call", "D"));
  ASSERT_EQ(aliases_ac_c, aliases_ac_d);

  ptr_set_t aliases_ac_r12_deref;
  ASSERT_NO_FATAL_FAILURE(query_deref_als(aliases_ac_r12_deref, ar, "after_call", "R12"));
  ASSERT_EQ(aliases_ac_r12_deref, aliases_ac_c);
}

TEST_F(summary_dstack_test, Call_2AliasesCallee_2AliasesCaller) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
"g:\n\
mov %r13, %r14\n\
ret\n\
\n\
f:\n\
//mov %rcx, %rcx\n\
//mov %rdx, %rdx\n\
je else\n\
mov %rcx, %r11\n\
jmp ite_end\n\
else:\n\
mov %rdx, %r11\n\
ite_end:\n\
mov %r11, (%r12)\n\
ret\n\
\n\
main:\n\
movq $41000, (%rcx)\n\
movq $42000, (%rdx)\n\
call f\n\
after_call: mov (%r12), %r12\n\
mov (%r12), %r13\n\
end: ret", false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R13", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({41000, 42000})));

  ptr_set_t aliases_ac_c;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_ac_c, ar, "after_call", "C"));
  ptr_set_t aliases_ac_d;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_ac_d, ar, "after_call", "D"));

  ptr_set_t aliases_ac_c_d;
  set_union(aliases_ac_c.begin(), aliases_ac_c.end(), aliases_ac_d.begin(), aliases_ac_d.end(), inserter(aliases_ac_c_d, aliases_ac_c_d.begin()));

  ptr_set_t aliases_ac_r12_deref;
  ASSERT_NO_FATAL_FAILURE(query_deref_als(aliases_ac_r12_deref, ar, "after_call", "R12"));
  ASSERT_EQ(aliases_ac_r12_deref, aliases_ac_c_d);
}

TEST_F(summary_dstack_test, Call_2AliasesForOneVariableCaller) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
"g:\n\
mov %r13, %r14\n\
ret\n\
\n\
f:\n\
mov %rax, (%rdx)\n\
ret\n\
\n\
main:\n\
mov %rcx, %rcx\n\
mov %rdx, %rdx\n\
start:\n\
je if_end\n\
mov %rcx, %rdx\n\
if_end:\n\
call f\n\
end:\n\
ret", false));

  ptr_set_t aliases_start_c;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_start_c, ar, "start", "C"));
  ASSERT_EQ(aliases_start_c.size(), 1);

  ptr_set_t aliases_start_d;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_start_d, ar, "start", "D"));
  ASSERT_EQ(aliases_start_d.size(), 1);

  ptr_set_t aliases_end_a;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_end_a, ar, "end", "A"));
  ASSERT_EQ(aliases_end_a.size(), 1);

  ptr_set_t aliases_start_c_end_deref;
  ASSERT_NO_FATAL_FAILURE(query_deref_als(aliases_start_c_end_deref, ar, "end", *aliases_start_c.begin()));
  ASSERT_EQ(aliases_start_c_end_deref.size(), 2);
  ASSERT_NE(aliases_start_c_end_deref.find(*aliases_end_a.begin()), aliases_start_c_end_deref.end());
  ASSERT_EQ(aliases_start_c_end_deref.find(*aliases_start_d.begin()), aliases_start_c_end_deref.end());
  ASSERT_EQ(aliases_start_c_end_deref.find(*aliases_start_c.begin()), aliases_start_c_end_deref.end());

  ptr_set_t aliases_start_d_end_deref;
  ASSERT_NO_FATAL_FAILURE(query_deref_als(aliases_start_d_end_deref, ar, "end", *aliases_start_d.begin()));
  ASSERT_EQ(aliases_start_d_end_deref.size(), 2);
  ASSERT_NE(aliases_start_d_end_deref.find(*aliases_end_a.begin()), aliases_start_d_end_deref.end());
  ASSERT_EQ(aliases_start_d_end_deref.find(*aliases_start_d.begin()), aliases_start_d_end_deref.end());
  ASSERT_EQ(aliases_start_d_end_deref.find(*aliases_start_c.begin()), aliases_start_d_end_deref.end());
}

TEST_F(summary_dstack_test, Call_2AliasesForOneVariableCallerWithOffsetInCaller) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
"f:\n\
mov %rax, (%rdx)\n\
ret\n\
\n\
main:\n\
mov %rcx, %rcx\n\
mov %rdx, %rdx\n\
start:\n\
je if_end\n\
mov %rcx, %rdx\n\
if_end:\n\
add $8, %rdx\n\
call f\n\
end:\n\
ret", false));

  ptr_set_t aliases_start_c;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_start_c, ar, "start", "C"));
  ASSERT_EQ(aliases_start_c.size(), 1);
  ptr aliases_start_c_only = *aliases_start_c.begin();
  ptr aliases_start_c_only_plus_8 = ptr(aliases_start_c_only.id, *aliases_start_c_only.offset + vs_finite::single(8));

  ptr_set_t aliases_start_d;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_start_d, ar, "start", "D"));
  ASSERT_EQ(aliases_start_d.size(), 1);
  ptr aliases_start_d_only = *aliases_start_d.begin();
  ptr aliases_start_d_only_plus_8 = ptr(aliases_start_d_only.id, *aliases_start_d_only.offset + vs_finite::single(8));

  ptr_set_t aliases_end_a;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_end_a, ar, "end", "A"));
  ASSERT_EQ(aliases_end_a.size(), 1);

  ptr_set_t aliases_start_c_end_deref;
  ASSERT_NO_FATAL_FAILURE(query_deref_als(aliases_start_c_end_deref, ar, "end", aliases_start_c_only_plus_8));
  ASSERT_EQ(aliases_start_c_end_deref.size(), 2);
  ASSERT_NE(aliases_start_c_end_deref.find(*aliases_end_a.begin()), aliases_start_c_end_deref.end());
  ASSERT_EQ(aliases_start_c_end_deref.find(aliases_start_d_only_plus_8), aliases_start_c_end_deref.end());
  ASSERT_EQ(aliases_start_c_end_deref.find(aliases_start_c_only_plus_8), aliases_start_c_end_deref.end());

  ptr_set_t aliases_start_d_end_deref;
  ASSERT_NO_FATAL_FAILURE(query_deref_als(aliases_start_d_end_deref, ar, "end", aliases_start_d_only_plus_8));
  ASSERT_EQ(aliases_start_d_end_deref.size(), 2);
  ASSERT_NE(aliases_start_d_end_deref.find(*aliases_end_a.begin()), aliases_start_d_end_deref.end());
  ASSERT_EQ(aliases_start_d_end_deref.find(aliases_start_d_only_plus_8), aliases_start_d_end_deref.end());
  ASSERT_EQ(aliases_start_d_end_deref.find(aliases_start_c_only_plus_8), aliases_start_d_end_deref.end());
}

TEST_F(summary_dstack_test, StructuralCompat) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
"main:\n\
je else\n\
mov %rbx, (%rax)\n\
jmp end\n\
else:\n\
mov %rcx, (%rax)\n\
end:\n\
ret", false));

  ptr_set_t aliases_b;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_b, ar, "end", "B"));
  ASSERT_EQ(aliases_b.size(), 1);
  ptr_set_t aliases_c;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_c, ar, "end", "C"));
  ASSERT_EQ(aliases_c.size(), 1);

  ptr_set_t aliases_a_deref;
  ASSERT_NO_FATAL_FAILURE(query_deref_als(aliases_a_deref, ar, "end", "A"));
  ASSERT_EQ(aliases_a_deref.size(), 2);
  ASSERT_NE(aliases_a_deref.find(*aliases_b.begin()), aliases_a_deref.end());
  ASSERT_NE(aliases_a_deref.find(*aliases_c.begin()), aliases_a_deref.end());

  ptr_set_t aliases_ip;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_ip, ar, "end", "IP"));
  ASSERT_EQ(aliases_ip.size(), 1);
}
