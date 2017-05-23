/*
 * dstack.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: Julian Kranz
 */

#include <algorithm>
#include <bjutil/binary/elf_provider.h>
#include <bjutil/gdsl_init.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/rreil/rreil.h>
#include <experimental/optional>
#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <summy/analysis/domains/util.h>
#include <summy/analysis/fixpoint.h>
#include <summy/big_step/analysis_dectran.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/jd_manager.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node_visitor.h>
#include <summy/rreil/id/memory_id.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/test/analysis/domains/common.h>
#include <summy/test/compile.h>

using namespace analysis;
using namespace analysis::api;
using namespace std;
using namespace gdsl::rreil;
using namespace cfg;
using namespace summy::rreil;

using namespace std;
using namespace std::experimental;
using namespace summy;

class summary_dstack_test : public ::testing::Test {
protected:
  summary_dstack_test() {}

  virtual ~summary_dstack_test() {}

  virtual void SetUp() {}

  virtual void TearDown() {}
};

static void query_val(vs_shared_t &r, _analysis_result &ar, string label, string arch_id_name,
  size_t offset, size_t size) {
  SCOPED_TRACE("query_val()");

  auto analy_r = ar.ds_analyzed->result();

  bool found;
  binary_provider::entry_t e;
  tie(found, e) = ar.elfp->symbol(label);
  ASSERT_TRUE(found);

  auto addr_it = ar.addr_node_map.find(e.address);
  ASSERT_NE(addr_it, ar.addr_node_map.end());

  ASSERT_GT(analy_r.result.size(), addr_it->second);

  lin_var *lv = new lin_var(make_variable(make_id(arch_id_name), offset));
//   cout << *analy_r.result[ar.addr_node_map[e.address]].at(0)->get_mstate() << endl;
  
  cout << hex << e.address << dec << endl;
  cout << ar.addr_node_map[e.address] << endl;
  
  cout << *analy_r.result[ar.addr_node_map[e.address]].at(0)->get_mstate() << endl;

  r = analy_r.result[ar.addr_node_map[e.address]].at(0)->get_mstate()->queryVal(lv, size);
  delete lv;
}

// static void query_eq(vs_shared_t &r, _analysis_result &ar, string label, string arch_id_first,
// string arch_id_second)
// {
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
// static void equal_structure(region_t const& a, region_t const& b) {
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

static void equal_structure(
  region_t const &cmp, _analysis_result &ar, string label, id_shared_t id) {
  SCOPED_TRACE("equal_structure()");

  auto analy_r = ar.ds_analyzed->result();

  bool found;
  binary_provider::entry_t e;
  tie(found, e) = ar.elfp->symbol(label);
  ASSERT_TRUE(found);

  auto addr_it = ar.addr_node_map.find(e.address);
  ASSERT_NE(addr_it, ar.addr_node_map.end());

  ASSERT_GT(analy_r.result.size(), addr_it->second);

  region_t const &rr =
    analy_r.result[ar.addr_node_map[e.address]].at(0)->get_mstate()->query_region_output(id);

  equal_structure(cmp, rr);
}

static void equal_structure(
  region_t const &cmp, _analysis_result &ar, string label, string arch_id_name) {
  SCOPED_TRACE("equal_structure()");

  id_shared_t id = shared_ptr<gdsl::rreil::id>(new arch_id(arch_id_name));
  equal_structure(cmp, ar, label, id);
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

  *mstate = analy_r.result[ar.addr_node_map[e.address]].at(0)->get_mstate();
}

static void query_deref(
  id_shared_t &id, _analysis_result &ar, summary_memory_state *mstate, ptr _ptr, size_t size) {
  SCOPED_TRACE("query_deref()");

  int64_t offset;
  value_set_visitor vsv;
  vsv._([&](vs_finite const *vsf) {
    if(!vsf->is_singleton()) throw string("query_deref_als(): need singleton alias set :-/");
    offset = *vsf->get_elements().begin();
  });
  _ptr.offset->accept(vsv);

  num_var *ptr_var = new num_var(_ptr.id);

  num_linear *derefed = mstate->dereference(ptr_var, 8 * offset, size);
  delete ptr_var;

  //  if(*derefed == *value_set::top) {
  //    FAIL() << "mstate->dereference yielded top :-(";
  //  }

  num_var *derefed_var = NULL;
  num_visitor nv;
  nv._([&](num_linear_term *nt) {
    ASSERT_EQ(nt->get_scale(), 1);
    num_visitor nv_inner;
    bool success = false;
    nv_inner._([&](num_linear_vs *vs) {
      ASSERT_EQ(*vs->get_value_set(), vs_finite::zero);
      success = true;
    });
    nt->get_next()->accept(nv_inner);
    if(success) derefed_var = nt->get_var();
  });
  derefed->accept(nv);
  ASSERT_NE(derefed_var, (void *)NULL);

  id = derefed_var->get_id();
  delete derefed;
}

static void query_deref_als(
  ptr_set_t &aliases, _analysis_result &ar, summary_memory_state *mstate, ptr _ptr) {
  SCOPED_TRACE("query_deref_als()");

  id_shared_t id;
  query_deref(id, ar, mstate, _ptr, 64);
  num_var *derefed_var = new num_var(id);
  aliases = mstate->queryAls(derefed_var);
  delete derefed_var;
}

static void query_deref_als(
  ptr_set_t &aliases, _analysis_result &ar, summary_memory_state *mstate, string arch_id_name) {
  SCOPED_TRACE("query_deref_als()");

  address *a = new address(64, make_linear(make_variable(make_id(arch_id_name), 0)));
  ptr_set_t addresses = mstate->queryAls(a);
  ptr const &p = unpack_singleton(addresses);

  query_deref_als(aliases, ar, mstate, p);

  delete a;
}

static void query_deref_als(
  ptr_set_t &aliases, _analysis_result &ar, string label, string arch_id_name) {
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

// static void query_deref_region(region_t &region, _analysis_result &ar, string label, string
// arch_id_name) {
//  SCOPED_TRACE("query_deref_region()");
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
//  //  cout << *analy_r.result[ar.addr_node_map[e.address]]->get_mstate() << endl;
//
//  address *a = new address(64, new lin_var(new variable(new arch_id(arch_id_name), 0)));
//  ptr_set_t aliases = analy_r.result[ar.addr_node_map[e.address]]->get_mstate()->queryAls(a);
//
//  ASSERT_EQ(aliases.size(), 1);
//  ptr const &_ptr = *aliases.begin();
//  ASSERT_EQ(*_ptr.offset, vs_finite::zero);
//
//  region = analy_r.result[ar.addr_node_map[e.address]]->get_mstate()->query_deref_output(_ptr.id);
//
//  delete a;
//}

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

  address *a = new address(64, make_linear(make_variable(make_id(arch_id_name), 0)));
  aliases = analy_r.result[ar.addr_node_map[e.address]].at(0)->get_mstate()->queryAls(a);

  delete a;
}

static void isTop(bool &result, _analysis_result &ar, string label) {
  SCOPED_TRACE("isTop()");

  auto analy_r = ar.ds_analyzed->result();

  bool found;
  binary_provider::entry_t e;
  tie(found, e) = ar.elfp->symbol(label);
  ASSERT_TRUE(found);

  auto addr_it = ar.addr_node_map.find(e.address);
  ASSERT_NE(addr_it, ar.addr_node_map.end());

  ASSERT_GT(analy_r.result.size(), addr_it->second);

  summary_memory_state *state = analy_r.result[ar.addr_node_map[e.address]].at(0)->get_mstate();

  summary_memory_state *top = state->copy();
  top->topify();

  result = (*state <= *top) && (*top <= *state);

  delete top;
}

static void assert_ptrs(optional<vs_shared_t> &offset, ptr_set_t &ptrs, bool expect_null,
  bool expect_bad, unsigned expect_allocs, unsigned expect_anon,
  optional<string> ptr_name = nullopt) {
  ASSERT_EQ(
    ptrs.size(), (expect_null ? 1 : 0) + (expect_bad ? 1 : 0) + expect_allocs + expect_anon);
  bool has_null = false;
  bool has_bad = false;
  unsigned allocs = 0;
  unsigned anon = 0;
  bool has_name = false;
  for(auto &ptr : ptrs) {
    offset = ptr.offset;
    summy::rreil::id_visitor idv;
    idv._([&](allocation_memory_id const *alloc_id) { allocs++; });
    idv._([&](ptr_memory_id const *pid) {
      anon++;
      if(ptr_name) {
        summy::rreil::id_visitor ptr_id_visitor;
        ptr_id_visitor._([&](numeric_id const *nid) {
          auto name = nid->get_name();
          if(name && name.value() == ptr_name.value()) has_name = true;
        });
        pid->get_id().accept(ptr_id_visitor);
      }
    });
    idv._([&](special_ptr const *pid) {
      if(*pid == *special_ptr::_nullptr)
        has_null = true;
      else if(*pid == *special_ptr::badptr)
        has_bad = true;
    });
    ptr.id->accept(idv);
  }
  ASSERT_EQ(has_null, expect_null);
  ASSERT_EQ(has_bad, expect_bad);
  ASSERT_EQ(allocs, expect_allocs);
  ASSERT_EQ(anon, expect_anon);
  ASSERT_EQ(has_name, ptr_name ? true : false);
}

static void assert_ptrs(ptr_set_t &ptrs, bool expect_null, bool expect_bad, unsigned expect_allocs,
  unsigned expect_anon, optional<string> ptr_name = nullopt) {
  optional<vs_shared_t> offset;
  assert_ptrs(offset, ptrs, expect_null, expect_bad, expect_allocs, expect_anon, ptr_name);
}

TEST_F(summary_dstack_test, FromDstack_Basics) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, "mov $99, %rax\n\
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
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, "mov $99, %eax\n\
   first: mov $20, %rax\n\
   second: nop\n"));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "first", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(99));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(20));

  {
    region_t cmp;
    cmp.insert(make_pair(0, field{32, numeric_id::generate()}));
    cmp.insert(make_pair(32, field{32, numeric_id::generate()}));
    equal_structure(cmp, ar, "first", "A");
  }
  {
    region_t cmp;
    cmp.insert(make_pair(0, field{64, numeric_id::generate()}));
    equal_structure(cmp, ar, "second", "A");
  }
}

TEST_F(summary_dstack_test, FieldSplitSameOffset) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, "mov $20, %rax\n\
   first: mov $99, %eax\n\
   second: nop\n"));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "first", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(20));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(99));

  {
    region_t cmp;
    cmp.insert(make_pair(0, field{64, numeric_id::generate()}));
    equal_structure(cmp, ar, "first", "A");
  }
  {
    region_t cmp;
    cmp.insert(make_pair(0, field{32, numeric_id::generate()}));
    cmp.insert(make_pair(32, field{32, numeric_id::generate()}));
    equal_structure(cmp, ar, "second", "A");
  }
}

TEST_F(summary_dstack_test, FieldSplitSameOffset2) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, "mov $20, %rax\n\
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
    cmp.insert(make_pair(0, field{64, numeric_id::generate()}));
    equal_structure(cmp, ar, "first", "A");
  }
  {
    region_t cmp;
    cmp.insert(make_pair(0, field{8, numeric_id::generate()}));
    cmp.insert(make_pair(8, field{56, numeric_id::generate()}));
    equal_structure(cmp, ar, "second", "A");
  }
}

TEST_F(summary_dstack_test, FieldSplitMiddle) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, "mov $20, %rax\n\
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
    cmp.insert(make_pair(0, field{64, numeric_id::generate()}));
    equal_structure(cmp, ar, "first", "A");
  }
  {
    region_t cmp;
    cmp.insert(make_pair(0, field{8, numeric_id::generate()}));
    cmp.insert(make_pair(8, field{8, numeric_id::generate()}));
    cmp.insert(make_pair(16, field{48, numeric_id::generate()}));
    equal_structure(cmp, ar, "second", "A");
  }
}

TEST_F(summary_dstack_test, OverlappingFieldMiddle) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, "mov $99, %ah\n\
   first: mov $20, %rax\n\
   second: nop\n"));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "first", "A", 8, 8));
  ASSERT_EQ(*r, vs_finite::single(99));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(20));

  {
    region_t cmp;
    cmp.insert(make_pair(8, field{8, numeric_id::generate()}));
    equal_structure(cmp, ar, "first", "A");
  }
  {
    region_t cmp;
    cmp.insert(make_pair(0, field{64, numeric_id::generate()}));
    equal_structure(cmp, ar, "second", "A");
  }
}

TEST_F(summary_dstack_test, Call) {
  // test_call.as
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, ".byte 0\n\
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
after_call: movq (%rax), %r13\n\
movq (%r13), %r13\n\
end: ret",
    false));

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
  // test21.as using aliases
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, ".byte 0\n\
g:\n\
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
end: ret",
    false));

  ptr_set_t aliases_before_call_r11;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_before_call_r11, ar, "before_call", "R11"));
  ASSERT_EQ(aliases_before_call_r11.size(), 2);
  ptr alias_before_call_r11 = unpack_singleton(aliases_before_call_r11);

  ptr_set_t aliases_r11;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_r11, ar, "end", "R11"));
  ASSERT_EQ(aliases_r11.size(), 2);
  ASSERT_EQ(unpack_singleton(aliases_r11), ptr(alias_before_call_r11.id, vs_finite::single(3)));

  ptr_set_t aliases_r12;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_r12, ar, "end", "R12"));
  ASSERT_EQ(aliases_r12.size(), 2);
  ASSERT_EQ(*unpack_singleton(aliases_r12).offset, vs_finite::single(11));
}

TEST_F(summary_dstack_test, 2Calls) {
  // test20.as using aliases
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, ".byte 0\n\
f:\n\
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
end: ret",
    false));

  ptr_set_t aliases_start_r14;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_start_r14, ar, "start", "R14"));
  ASSERT_EQ(aliases_start_r14.size(), 2);

  ptr_set_t aliases_start_r15;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_start_r15, ar, "start", "R15"));
  ASSERT_EQ(aliases_start_r15.size(), 2);

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
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, ".byte 0\n\
g:\n\
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
end: ret",
    false));

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
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, "g:\n\
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
end: ret",
    false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R13", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({41000, 42000})));

  ptr_set_t aliases_ac_c;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_ac_c, ar, "after_call", "C"));
  ptr_set_t aliases_ac_d;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_ac_d, ar, "after_call", "D"));

  ptr_set_t aliases_ac_c_d;
  set_union(aliases_ac_c.begin(), aliases_ac_c.end(), aliases_ac_d.begin(), aliases_ac_d.end(),
    inserter(aliases_ac_c_d, aliases_ac_c_d.begin()));

  ptr_set_t aliases_ac_r12_deref;
  ASSERT_NO_FATAL_FAILURE(query_deref_als(aliases_ac_r12_deref, ar, "after_call", "R12"));
  ASSERT_EQ(aliases_ac_r12_deref, aliases_ac_c_d);
}

TEST_F(summary_dstack_test, Call_2AliasesForOneVariableCaller) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, ".byte 0\n\
g:\n\
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
ret",
    false));

  ptr_set_t aliases_start_c;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_start_c, ar, "start", "C"));
  ASSERT_EQ(aliases_start_c.size(), 2);

  ptr_set_t aliases_start_d;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_start_d, ar, "start", "D"));
  ASSERT_EQ(aliases_start_d.size(), 2);

  ptr_set_t aliases_end_a;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_end_a, ar, "end", "A"));
  ASSERT_EQ(aliases_end_a.size(), 2);

  ptr_set_t aliases_start_c_end_deref;
  ASSERT_NO_FATAL_FAILURE(
    query_deref_als(aliases_start_c_end_deref, ar, "end", unpack_singleton(aliases_start_c)));
  ASSERT_EQ(aliases_start_c_end_deref.size(), 3);
  ASSERT_NE(aliases_start_c_end_deref.find(unpack_singleton(aliases_end_a)),
    aliases_start_c_end_deref.end());
  ASSERT_EQ(aliases_start_c_end_deref.find(unpack_singleton(aliases_start_d)),
    aliases_start_c_end_deref.end());
  ASSERT_EQ(aliases_start_c_end_deref.find(unpack_singleton(aliases_start_c)),
    aliases_start_c_end_deref.end());

  ptr_set_t aliases_start_d_end_deref;
  ASSERT_NO_FATAL_FAILURE(
    query_deref_als(aliases_start_d_end_deref, ar, "end", unpack_singleton(aliases_start_c)));
  ASSERT_EQ(aliases_start_d_end_deref.size(), 3);
  ASSERT_NE(aliases_start_d_end_deref.find(unpack_singleton(aliases_end_a)),
    aliases_start_d_end_deref.end());
  ASSERT_EQ(aliases_start_d_end_deref.find(unpack_singleton(aliases_start_d)),
    aliases_start_d_end_deref.end());
  ASSERT_EQ(aliases_start_d_end_deref.find(unpack_singleton(aliases_start_c)),
    aliases_start_d_end_deref.end());
}

TEST_F(summary_dstack_test, Call_2AliasesForOneVariableCallerWithOffsetInCaller) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, ".byte 0\n\
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
add $8, %rdx\n\
call f\n\
end:\n\
ret",
    false));

  ptr_set_t aliases_start_c;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_start_c, ar, "start", "C"));
  ASSERT_EQ(aliases_start_c.size(), 2);
  ptr aliases_start_c_only = unpack_singleton(aliases_start_c);
  ptr aliases_start_c_only_plus_8 =
    ptr(aliases_start_c_only.id, *aliases_start_c_only.offset + vs_finite::single(8));

  ptr_set_t aliases_start_d;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_start_d, ar, "start", "D"));
  ASSERT_EQ(aliases_start_d.size(), 2);
  ptr aliases_start_d_only = unpack_singleton(aliases_start_d);
  ptr aliases_start_d_only_plus_8 =
    ptr(aliases_start_d_only.id, *aliases_start_d_only.offset + vs_finite::single(8));

  ptr_set_t aliases_end_a;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_end_a, ar, "end", "A"));
  ASSERT_EQ(aliases_end_a.size(), 2);

  ptr_set_t aliases_start_c_end_deref;
  ASSERT_NO_FATAL_FAILURE(
    query_deref_als(aliases_start_c_end_deref, ar, "end", aliases_start_c_only_plus_8));
  ASSERT_EQ(aliases_start_c_end_deref.size(), 3);
  ASSERT_NE(aliases_start_c_end_deref.find(unpack_singleton(aliases_end_a)),
    aliases_start_c_end_deref.end());
  ASSERT_EQ(
    aliases_start_c_end_deref.find(aliases_start_d_only_plus_8), aliases_start_c_end_deref.end());
  ASSERT_EQ(
    aliases_start_c_end_deref.find(aliases_start_c_only_plus_8), aliases_start_c_end_deref.end());

  ptr_set_t aliases_start_d_end_deref;
  ASSERT_NO_FATAL_FAILURE(
    query_deref_als(aliases_start_d_end_deref, ar, "end", aliases_start_d_only_plus_8));
  ASSERT_EQ(aliases_start_d_end_deref.size(), 3);
  ASSERT_NE(aliases_start_d_end_deref.find(unpack_singleton(aliases_end_a)),
    aliases_start_d_end_deref.end());
  ASSERT_EQ(
    aliases_start_d_end_deref.find(aliases_start_d_only_plus_8), aliases_start_d_end_deref.end());
  ASSERT_EQ(
    aliases_start_d_end_deref.find(aliases_start_c_only_plus_8), aliases_start_d_end_deref.end());
}

TEST_F(summary_dstack_test, StructuralCompat) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, "main:\n\
je else\n\
mov %rbx, (%rax)\n\
jmp end\n\
else:\n\
mov %rcx, (%rax)\n\
end:\n\
ret",
    false));

  ptr_set_t aliases_b;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_b, ar, "end", "B"));
  ASSERT_EQ(aliases_b.size(), 2);
  ptr_set_t aliases_c;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_c, ar, "end", "C"));
  ASSERT_EQ(aliases_c.size(), 2);

  ptr_set_t aliases_a_deref;
  ASSERT_NO_FATAL_FAILURE(query_deref_als(aliases_a_deref, ar, "end", "A"));
  ASSERT_EQ(aliases_a_deref.size(), 3);
  ASSERT_NE(aliases_a_deref.find(unpack_singleton(aliases_b)), aliases_a_deref.end());
  ASSERT_NE(aliases_a_deref.find(unpack_singleton(aliases_c)), aliases_a_deref.end());

  ptr_set_t aliases_ip;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_ip, ar, "end", "IP"));
  ASSERT_EQ(aliases_ip.size(), 1);
}

TEST_F(summary_dstack_test, SummaryAppStructuralConflict1) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, ".byte 0\n\
f:\n\
mov (%rax), %r11\n\
mov %r12, (%r11)\n\
movb $22, (%rbx)\n\
ret\n\
\n\
main:\n\
mov %rcx, %rax\n\
mov %rcx, %rbx\n\
mov %r13, (%rax)\n\
call f\n\
ac: mov $42, %rax\n\
end: ret",
    false));

  bool ac_top;
  isTop(ac_top, ar, "ac");
  ASSERT_TRUE(ac_top);

  bool end_top;
  isTop(end_top, ar, "end");
  ASSERT_FALSE(end_top);

  //  ptr_set_t aliases_r13_deref;
  //  ASSERT_NO_FATAL_FAILURE(query_deref_als(aliases_r13_deref, ar, "end", "R13"));
  //  ASSERT_EQ(aliases_r13_deref.size(), 1);
  //
  //  ptr_set_t aliases_r12;
  //  ASSERT_NO_FATAL_FAILURE(query_als(aliases_r12, ar, "end", "R12"));
  //  ASSERT_EQ(aliases_r12.size(), 1);
  //
  //  ASSERT_EQ(aliases_r13_deref, aliases_r12);
  //
  //  {
  //    region_t cmp;
  //    cmp.insert(make_pair(0, field{8, numeric_id::generate()}));
  //    cmp.insert(make_pair(8, field{56, numeric_id::generate()}));
  //
  //    region_t region;
  //    query_deref_region(region, ar, "end", "A");
  //
  //    equal_structure(cmp, region);
  //  }
  //
  //  {
  //    region_t cmp;
  //    cmp.insert(make_pair(0, field{8, numeric_id::generate()}));
  //    cmp.insert(make_pair(8, field{56, numeric_id::generate()}));
  //
  //    region_t region;
  //    query_deref_region(region, ar, "end", "B");
  //
  //    equal_structure(cmp, region);
  //  }
}

TEST_F(summary_dstack_test, SummaryAppStructuralConflict2) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, ".byte 0\n\
f:\n\
mov (%rbx), %r11\n\
mov %r12, (%r11)\n\
movb $22, (%rax)\n\
ret\n\
\n\
main:\n\
mov %rcx, %rax\n\
mov %rcx, %rbx\n\
mov %r13, (%rbx)\n\
call f\n\
end: ret",
    false));

  bool end_top;
  isTop(end_top, ar, "end");
  ASSERT_TRUE(end_top);

  //  ptr_set_t aliases_r13_deref;
  //  ASSERT_NO_FATAL_FAILURE(query_deref_als(aliases_r13_deref, ar, "end", "R13"));
  //  ASSERT_EQ(aliases_r13_deref.size(), 1);
  //
  //  ptr_set_t aliases_r12;
  //  ASSERT_NO_FATAL_FAILURE(query_als(aliases_r12, ar, "end", "R12"));
  //  ASSERT_EQ(aliases_r12.size(), 1);
  //
  //  ASSERT_EQ(aliases_r13_deref, aliases_r12);
  //
  //  {
  //    region_t cmp;
  //    cmp.insert(make_pair(0, field{8, numeric_id::generate()}));
  //    cmp.insert(make_pair(8, field{56, numeric_id::generate()}));
  //
  //    region_t region;
  //    query_deref_region(region, ar, "end", "A");
  //
  //    equal_structure(cmp, region);
  //  }
  //
  //  {
  //    region_t cmp;
  //    cmp.insert(make_pair(0, field{8, numeric_id::generate()}));
  //    cmp.insert(make_pair(8, field{56, numeric_id::generate()}));
  //
  //    region_t region;
  //    query_deref_region(region, ar, "end", "B");
  //
  //    equal_structure(cmp, region);
  //  }
}

// TEST_F(summary_dstack_test, SummaryAppStructuralConflict3) {
//  _analysis_result ar;
//  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
//"\f:\n\
//mov %r11, (%rax)\n\
//mov %r12, 4(%rbx)\n\
//ret\n\
//\n\
//main:\n\
//mov %rcx, %rax\n\
//mov %rcx, %rbx\n\
//call f\n\
//end: ret", false));
//
//  {
//    region_t cmp;
//    cmp.insert(make_pair(0, field { 32, numeric_id::generate() }));
//    cmp.insert(make_pair(32, field { 64, numeric_id::generate() }));
//
//    region_t region;
//    query_deref_region(region, ar, "end", "A");
//
//    equal_structure(cmp, region);
//  }
//
//  {
//    region_t cmp;
//    cmp.insert(make_pair(0, field { 32, numeric_id::generate() }));
//    cmp.insert(make_pair(32, field { 64, numeric_id::generate() }));
//
//    region_t region;
//    query_deref_region(region, ar, "end", "B");
//
//    equal_structure(cmp, region);
//  }
//}

TEST_F(summary_dstack_test, CReturnFunctionPointers) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_c(ar, "int g() {\n\
  return 42;\n\
}\n\
\n\
int h() {\n\
  return 99;\n\
}\n\
\n\
typedef int (*f_t)();\n\
\n\
f_t __attribute__ ((noinline)) f(char x) {\n\
  if(x)\n\
    return h;\n\
  else\n\
    return g;\n\
}\n\
\n\
int main(int argc, char **argv) {\n\
  int (*fp)();\n\
  fp = f(argc);\n\
\n\
  long long x = fp();\n\
\n\
  __asm volatile ( \"mov %0, %%r11\"\n\
    : \"=a\" (x)\n\
    : \"a\" (x)\n\
    : \"r11\");\n\
  __asm volatile ( \"end:\");\n\
  return x;\n\
}",
    false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 64));
  ASSERT_EQ(*r, shared_ptr<vs_finite>(new vs_finite({42, 99})));
}

TEST_F(summary_dstack_test, CSetFunctionPointerRef) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_c(ar, "\n\
int h() {\n\
  return 122;\n\
}\n\
\n\
int __attribute__ ((noinline)) f(int (**fp)()) {\n\
  *fp = h;\n\
}\n\
\n\
int main(void) {\n\
  int (*fp)();\n\
  f(&fp);\n\
  long long x = fp();\n\
\n\
  __asm volatile ( \"mov %0, %%r11\"\n\
    : \"=a\" (x)\n\
    : \"a\" (x)\n\
    : \"r11\");\n\
  __asm volatile ( \"end:\");\n\
  return 0;\n\
}",
    false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(122));
}

TEST_F(summary_dstack_test, CppVirtualFunctionCall) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_cpp(ar, "\n\
#include <stdio.h>\n\
#include <stdlib.h>\n\
#include <string>\n\
\n\
struct hugo {\n\
  virtual int foo() {\n\
    return 99;\n\
  }\n\
  virtual ~hugo() { }\n\
};\n\
\n\
struct inge : public hugo {\n\
  virtual int foo() {\n\
    return 42;\n\
  }\n\
  virtual ~inge() { }\n\
};\n\
\n\
int main(int argc, char **argv) {\n\
  void *addr;\n\
\n\
  __asm volatile ( \"mov %%r11, %0\"\n\
    : \"=a\" (addr)\n\
    : \"a\" (addr)\n\
    : );\n\
\n\
  hugo *h = new (addr) inge();\n\
\n\
  long long x = h->foo();\n\
\n\
  __asm volatile ( \"mov %0, %%r11\"\n\
    : \"=a\" (x)\n\
    : \"a\" (x)\n\
    : \"r11\");\n\
\n\
  __asm volatile ( \"end:\" );\n\
\n\
  return x;\n\
}",
    false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(42));
}

TEST_F(summary_dstack_test, CppVirtualFunctionCallReturnObjectHeap) {
  // test_vcall_returnobjheap.cpp
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_cpp(ar, "\n\
struct A { virtual long f() = 0; };\n\
\n\
struct B : public A {\n\
  long f() { return 444; }\n\
};\n\
\n\
A *g() {\n\
  return new B();\n\
}\n\
\n\
int main(void) {\n\
  A *b = g();\n\
  long x = b->f();\n\
  \n\
  __asm volatile ( \"movq %0, %%r11\"\n\
    : \"=a\" (x)\n\
    : \"a\" (x)\n\
    : \"r11\");\n\
\n\
  __asm volatile ( \"end:\");\n\
  return 0;\n\
}",
    false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(444));
}

TEST_F(summary_dstack_test, CppVirtualFunctionCallGlobObject) {
  // test_vcall_returnobjheap.cpp
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_cpp(ar, "struct A { virtual long f() = 0; };\n\
\n\
struct B : public A {\n\
  long f() { return 279; }\n\
};\n\
\n\
A *p;\n\
\n\
void g() {\n\
  p = new B();\n\
}\n\
\n\
int main(void) {\n\
  g();\n\
  long x = p->f();\n\
  \n\
  __asm volatile ( \"movq %0, %%r11\"\n\
    : \"=a\" (x)\n\
    : \"a\" (x)\n\
    : \"r11\");\n\
\n\
  __asm volatile ( \"end:\");\n\
  return 0;\n\
}",
    false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(279));
}

TEST_F(summary_dstack_test, FunctionPointerParamter1) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_c(ar, "\n\
int f() {\n\
  return 57;\n\
}\n\
int g(int (*fp)(void)) {\n\
  return fp();\n\
}\n\
int main() {\n\
  int x = g(&f);\n\
  __asm volatile ( \"movl %0, %%r11d\"\n\
  : \"=a\" (x)\n\
  : \"a\" (x)\n\
  : \"r11\");\n\
\n\
   __asm volatile ( \"end:\" );\n\
     return 0;\n\
}",
    false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 32));
  ASSERT_EQ(*r, vs_finite::single(57));
}

TEST_F(summary_dstack_test, FunctionPointerParamter2) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_c(ar, "\n\
int f() {\n\
  return 44;\n\
}\n\
\n\
int h(int (*fp)(void)) {\n\
  return fp();\n\
}\n\
\n\
int g(int (*fp)(void)) {\n\
  return h(fp);\n\
}\n\
int main() {\n\
  int x = g(&f);\n\
  __asm volatile ( \"movl %0, %%r11d\"\n\
  : \"=a\" (x)\n\
  : \"a\" (x)\n\
  : \"r11\");\n\
\n\
  __asm volatile ( \"end:\" );\n\
  return 0;\n\
}",
    false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 32));
  ASSERT_EQ(*r, vs_finite::single(44));
}

TEST_F(summary_dstack_test, FunctionPointerParamter3) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_c(ar, "\n\
int f() {\n\
  return 77;\n\
}\n\
\n\
int h(int (*fp)(void)) {\n\
  return fp();\n\
}\n\
\n\
int g(int (*fp)(int(*)(void))) {\n\
  return fp(&f);\n\
}\n\
int main() {\n\
  int x = g(&h);\n\
  __asm volatile ( \"movl %0, %%r11d\"\n\
  : \"=a\" (x)\n\
  : \"a\" (x)\n\
  : \"r11\");\n\
\n\
  __asm volatile ( \"end:\" );\n\
  return 0;\n\
}",
    false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 32));
  ASSERT_EQ(*r, vs_finite::single(77));
}

TEST_F(summary_dstack_test, FunctionPointerParamter4) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_c(ar, "\n\
int f() {\n\
  return 42;\n\
}\n\
\n\
int q() {\n\
  return 99;\n\
}\n\
\n\
int h(int (*fp)(void)) {\n\
  return fp();\n\
}\n\
\n\
int g(int (*fp)(int(*)(void))) {\n\
  return fp(&f);\n\
}\n\
int main() {\n\
  int x = g(&h) + h(&q);\n\
  __asm volatile ( \"movl %0, %%r11d\"\n\
  : \"=a\" (x)\n\
  : \"a\" (x)\n\
  : \"r11\");\n\
\n\
__asm volatile ( \"end:\" );\n\
  return 0;\n\
}",
    false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 32));
  ASSERT_EQ(*r, make_shared<vs_finite>(vs_finite::elements_t{84, 141, 198}));
}

TEST_F(summary_dstack_test, LoopWideningNarrowing) {
  // test23-loop.as
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, "main:\n\
mov $0, %r11\n\
head:\n\
cmp $100, %r11\n\
je end\n\
inc %r11\n\
jmp head\n\
end:\n\
ret",
    true));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(100));

  /*
   * There are 4 to 6 iterations depending on whether the necessity for propagation
   * is checked only at back and call edges, additionally only for non-join points or
   * for every edge.
   */
  ASSERT_EQ(ar.max_it, 5);
}

TEST_F(summary_dstack_test, Malloc) {
  // test_malloc.c
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state(ar, "#include <stdlib.h>\n\
\n\
int main(void) {\n\
  __asm volatile ( \"before_malloc:\" );\n\
  int *x = malloc(4);\n\
  __asm volatile ( \"movq %0, %%r11\"\n\
  : \"=a\" (x)\n\
  : \"a\" (x)\n\
  : \"r11\");\n\
  __asm volatile ( \"after_malloc:\" );\n\
  return 0;\n\
}",
    C, false, 0, false));

  ptr_set_t sp_before_malloc;
  ASSERT_NO_FATAL_FAILURE(query_als(sp_before_malloc, ar, "before_malloc", "SP"));
  ptr_set_t sp_after_malloc;
  ASSERT_NO_FATAL_FAILURE(query_als(sp_after_malloc, ar, "after_malloc", "SP"));
  ASSERT_EQ(sp_before_malloc, sp_after_malloc);

  ptr_set_t r11_after_malloc;
  ASSERT_NO_FATAL_FAILURE(query_als(r11_after_malloc, ar, "after_malloc", "R11"));

  ptr alloc_ptr = unpack_singleton(r11_after_malloc);
  ASSERT_EQ(*alloc_ptr.offset, vs_finite::zero);

  summy::rreil::id_visitor idv;
  bool is_alloc = false;
  idv._([&](allocation_memory_id const *alloc_id) { is_alloc = true; });
  alloc_ptr.id->accept(idv);
  ASSERT_TRUE(is_alloc);
}

TEST_F(summary_dstack_test, MallocInitList) {
  // test_malloc2.c
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state(ar, "\n\
#include <stdlib.h>\n\
\n\
struct node;\n\
\n\
struct node {\n\
  struct node *next;\n\
};\n\
\n\
int main(int argc, char **argv) {\n\
  struct node *head = malloc(sizeof(struct node));\n\
  __asm volatile ( \"after_head_malloc:\" );\n\
  head->next = NULL;\n\
  for(int i = 0; i < argc; i++) {\n\
    __asm volatile ( \"before_next_malloc:\" );\n\
    struct node *next = malloc(sizeof(struct node));\n\
    __asm volatile ( \"after_next_malloc:\" );\n\
    next->next = head;\n\
    head = next;\n\
  }\n\
  return (int)head;\n\
}",
    C, false, 1, false));

  ptr_set_t a_after_head_malloc;
  ASSERT_NO_FATAL_FAILURE(query_als(a_after_head_malloc, ar, "after_head_malloc", "A"));
  assert_ptrs(a_after_head_malloc, true, false, 1, 0);

  ptr_set_t a_after_next_malloc;
  ASSERT_NO_FATAL_FAILURE(query_als(a_after_next_malloc, ar, "after_next_malloc", "A"));
  assert_ptrs(a_after_next_malloc, true, true, 1, 0);

  ASSERT_EQ(ar.max_it, 4);
}

TEST_F(summary_dstack_test, ListTraverse) {
  // test_malloc2.c
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state(ar, "\n\
#include <stdlib.h>\n\
\n\
struct node;\n\
\n\
struct node {\n\
  struct node *next;\n\
};\n\
\n\
int main(int argc, char **argv) {\n\
  struct node head;\n\
  struct node *last = head.next;\n\
  for(int i = 0; i < argc; i++) {\n\
    __asm volatile ( \"movq %0, %%r11\"\n\
    : \"=a\" (last)\n\
    : \"a\" (last)\n\
    : \"r11\");\n\
    __asm volatile ( \"before_reassignment:\" );\n\
    last = last->next;\n\
    __asm volatile ( \"movq %0, %%r11\"\n\
    : \"=a\" (last)\n\
    : \"a\" (last)\n\
    : \"r11\");\n\
    __asm volatile ( \"after_reassignment:\" );\n\
  }\n\
  return (int)last;\n\
}",
    C, false, 1, false));

  ptr_set_t r11_before_reassignment;
  ASSERT_NO_FATAL_FAILURE(query_als(r11_before_reassignment, ar, "before_reassignment", "R11"));
  assert_ptrs(r11_before_reassignment, false, true, 0, 1);

  ptr_set_t r11_after_reassignment;
  ASSERT_NO_FATAL_FAILURE(query_als(r11_after_reassignment, ar, "after_reassignment", "R11"));
  assert_ptrs(r11_after_reassignment, false, true, 0, 1);

  ASSERT_EQ(ar.max_it, 4);
}

TEST_F(summary_dstack_test, Ite1) {
  // test_ite1.as
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, "\n\
jne else\n\
movq $9999, %rax\n\
jmp end\n\
else:\n\
end:\n\
nop\n",
    false));

  ptr_set_t aliases_end_a;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases_end_a, ar, "end", "A"));
  optional<vs_shared_t> offset;
  assert_ptrs(offset, aliases_end_a, true, true, 0, 1, string("A_q"));
  ASSERT_EQ(*offset.value(), shared_ptr<value_set>(new vs_finite({0, 9999})));
}

TEST_F(summary_dstack_test, TestFormerNoTermination) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar, "callq 0x0"));
}

TEST_F(summary_dstack_test, Tabulation1) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_c(ar, "\n\
#include <stdlib.h>\n\
\n\
int f1() {\n\
return 42;\n\
}\n\
\n\
int g1() {\n\
return 99;\n\
}\n\
\n\
int f2() {\n\
return 86;\n\
}\n\
\n\
int g2() {\n\
return 0;\n\
}\n\
\n\
int h(int (*fp0)(void), int (*fp1)(void)) {\n\
return fp0() + fp1();\n\
}\n\
\n\
int main(int argc) {\n\
int (*fp0)(void);\n\
int (*fp1)(void);\n\
if(argc == 0) {\n\
  fp0 = &f1;\n\
  fp1 = &g1;\n\
} else {\n\
  fp0 = &f2;\n\
  fp1 = &g2;\n\
}\n\
register int x = h(fp0, fp1);\n\
__asm volatile ( \"movl %0, %%r12d\"\n\
: \"=a\" (x)\n\
: \"a\" (x)\n\
: \"r12\");\n\
register int y = h(&f1, &g1);\n\
__asm volatile ( \"movl %0, %%r11d\"\n\
 : \"=a\" (y)\n\
 : \"a\" (y)\n\
 : \"r11\");\n\
__asm volatile ( \"jmp test\" );\n\
__asm volatile ( \"test: nop\" );\n\
return 0;\n\
}",
    true, true));
  
  ofstream dot_noa_fs;
  dot_noa_fs.open("output_noa_test.dot", ios::out);
  ar.dt->get_cfg().dot(dot_noa_fs, [&](::cfg::node &n, ostream &out) {
    if(n.get_id() == 33 || n.get_id() == 42) {
      //out << n.get_id() << " [label=\"" << n.get_id() << "\n" << *ds.get(n.get_id()) << "\"]";
      out << n.get_id() << " [label=\"" << n.get_id() << "\n";
      for(auto ctx_mapping : ar.ds_analyzed->get_ctxful(n.get_id()))
        out << "CTX: " << ctx_mapping.first << "\t" << *ctx_mapping.second << endl;
      
      
      out << "\"]";
    }
    
    
    //            out << n.get_id() << " [label=\"" << n.get_id() << " ~ " <<
    //            *jd_man.address_of(n.get_id()) << "\"]";
    else
      n.dot(out);
  });
  dot_noa_fs.close();

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "test", "R11", 0, 32));
  ASSERT_EQ(*r, vs_finite::single(141));

  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "test", "R12", 0, 32));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({42, 86, 141, 185})));
}
