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
#include <summy/rreil/id/numeric_id.h>
#include <summy/cfg/bfs_iterator.h>
#include <gtest/gtest.h>
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

  _analysis_result() {
    ds_analyzed = NULL;
    elfp = NULL;
  }

  ~_analysis_result() {
    delete ds_analyzed;
    delete elfp;
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

  g.set_code(compiled.data(), compiled.size(), 0);

  dectran dt(g, gdsl_optimize);
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

static void equal_structure(region_t const& a, region_t const& b) {
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
  tie(found, e) = ar.elfp->entry(label);
  ASSERT_TRUE(found);

  auto addr_it = ar.addr_node_map.find(e.address);
  ASSERT_NE(addr_it, ar.addr_node_map.end());

  ASSERT_GT(analy_r.result.size(), addr_it->second);

  id_shared_t id = shared_ptr<gdsl::rreil::id>(new arch_id(arch_id_name));
  region_t const& rr = analy_r.result[ar.addr_node_map[e.address]]->query_region(id);

  equal_structure(cmp, rr);
}

static void query_als(ptr_set_t &aliases, _analysis_result &ar, string label, string arch_id_name) {
  SCOPED_TRACE("query_als()");

  auto analy_r = ar.ds_analyzed->result();

  bool found;
  binary_provider::entry_t e;
  tie(found, e) = ar.elfp->entry(label);
  ASSERT_TRUE(found);

  auto addr_it = ar.addr_node_map.find(e.address);
  ASSERT_NE(addr_it, ar.addr_node_map.end());

  ASSERT_GT(analy_r.result.size(), addr_it->second);

  address *a = new address(64, new lin_var(new variable(new arch_id(arch_id_name), 0)));
  aliases = analy_r.result[ar.addr_node_map[e.address]]->queryAls(a);
  delete a;
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

TEST_F(dstack_test, TwoFieldsReplaceOneField) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "mov $99, %rax\n\
   first: mov $20, %eax\n\
   second: nop\n"));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "first", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(99));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "A", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(20));

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

TEST_F(dstack_test, MemorySimpleAddition) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "movq $144, (%rax)\n\
   movq (%rax), %rbx\n\
   add $10, %rbx\n\
   end: nop\n"));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "B", 0, 64));
  ASSERT_EQ(*r, vs_finite::single(154));
}

TEST_F(dstack_test, IfThenElseMemory) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "mov $99, %rbx\n\
    jne else\n\
    movq $10, (%rcx)\n\
    mov $40, %rax\n\
    add %rax, (%rcx)\n\
    jmp eite\n\
    else:\n\
    movq $0, (%rcx)\n\
    mov $10, %rax\n\
    add %rbx, %rax\n\
    eite:\n\
    mov (%rcx), %rbx\n\
    end: nop"));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "A", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 40, 109 })));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "B", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 50, 0 })));
}

TEST_F(dstack_test, IfThenElseAssumptions) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "cmp $10, %rax\n\
    jge else\n\
    less:\n\
    cmp $5, %rax\n\
    jle else\n\
    less_greater:\n\
    mov $40, %rbx\n\
    jmp eite\n\
    else:\n\
    mov $10, %rbx\n\
    eite:\n\
    jmp end\n\
    end:\n\
    ret", true));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "less", "A", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_open(DOWNWARD, 9)));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "less_greater", "A", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 6, 7, 8, 9 })));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "A", 0, 64));
  ASSERT_EQ(*r, value_set::top);
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "B", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 40, 10 })));

  /*
   * Todo: Another test that checks for bottom if the value of rax is known
   */
}

TEST_F(dstack_test, IfThenElseMemoryUndefWeakUpdate) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "jne else\n\
    mov %rbx, %rax\n\
    jmp eite\n\
    else:\n\
    mov %rcx, %rax\n\
    eite:\n\
    movq $42, (%rax)\n\
    movq (%rbx), %rdx\n\
    addq (%rcx), %rdx\n\
    movq (%rax), %r11\n\
    end: nop", false));

  ptr_set_t aliases;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases, ar, "eite", "A"));
  ASSERT_EQ(aliases.size(), 2);

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "D", 0, 64));
  ASSERT_EQ(*r, value_set::top);
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 64));
  ASSERT_EQ(*r, value_set::top);
}

TEST_F(dstack_test, IfThenElseMemoryDefWeakUpdate) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "movq $1, (%rbx)\n\
    movq $2, (%rcx)\n\
    jne else\n\
    mov %rbx, %rax\n\
    jmp eite\n\
    else:\n\
    mov %rcx, %rax\n\
    eite:\n\
    movq $42, (%rax)\n\
    movq (%rbx), %rdx\n\
    first:\n\
    movq (%rcx), %rdx\n\
    second:\n\
    movq (%rax), %rdx\n\
    end: nop", false));

  ptr_set_t aliases;
  ASSERT_NO_FATAL_FAILURE(query_als(aliases, ar, "eite", "A"));
  ASSERT_EQ(aliases.size(), 2);

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "first", "D", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 1, 42 })));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "D", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 2, 42 })));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "D", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 1, 2, 42 })));
}

TEST_F(dstack_test, IfThenElseMemoryOffsetWeakUpdate) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "jne else\n\
    mov $8, %rax\n\
    jmp eite\n\
    else:\n\
    mov $15, %rax\n\
    eite:\n\
    movq $42, (%rbx,%rax)\n\
    movq 8(%rbx), %r11\n\
    movq 15(%rbx), %r12\n\
    end: nop", false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 64));
  ASSERT_EQ(*r, value_set::top);
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R12", 0, 64));
  ASSERT_EQ(*r, value_set::top);
}

TEST_F(dstack_test, IfThenElseMemoryOffsetStrongUpdate) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "jne else\n\
    mov $16, %rax\n\
    jmp eite\n\
    else:\n\
    mov $16, %rax\n\
    eite:\n\
    movq $42, (%rbx,%rax)\n\
    movq 8(%rbx), %r11\n\
    movq 16(%rbx), %r12\n\
    end: nop", false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 64));
  ASSERT_EQ(*r, value_set::top);
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R12", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 42 })));
}

TEST_F(dstack_test, IfThenElseMemoryOffsetWeakUpdatePredef) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "movq $0, 8(%rbx)\n\
    movq $1, 16(%rbx)\n\
    jne else\n\
    mov $8, %rax\n\
    jmp eite\n\
    else:\n\
    mov $16, %rax\n\
    eite:\n\
    movq $42, (%rbx,%rax)\n\
    movq 8(%rbx), %r11\n\
    movq 16(%rbx), %r12\n\
    end: nop", false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 0, 42 })));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R12", 0, 64));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 1, 42 })));
}

TEST_F(dstack_test, IfThenElseMemoryOffsetWeakUpdatePredefOverlap) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "movq $0, 8(%rbx)\n\
    movq $1, 16(%rbx)\n\
    jne else\n\
    mov $8, %rax\n\
    jmp eite\n\
    else:\n\
    mov $15, %rax\n\
    eite:\n\
    movq $42, (%rbx,%rax)\n\
    movq 8(%rbx), %r11\n\
    movq 16(%rbx), %r12\n\
    end: nop", false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R11", 0, 64));
  ASSERT_EQ(*r, value_set::top);
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "R12", 0, 64));
  ASSERT_EQ(*r, value_set::top);;
}

TEST_F(dstack_test, PointerArithmetic) {
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state_asm(ar,
   "movb $99, (%rbx)\n\
    movb $100, 1(%rbx)\n\
    jne else\n\
    mov %rbx, %rax\n\
    jmp eite\n\
    else:\n\
    mov %rbx, %rax\n\
    add $1, %rax\n\
    eite:\n\
    movb (%rax), %cl\n\
    first: addb $42, (%rax)\n\
    movb (%rax), %cl\n\
    second: movb (%rbx), %cl\n\
    third: movb 1(%rbx), %cl\n\
    end: nop", false));

  vs_shared_t r;
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "first", "C", 0, 8));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 99, 100 })));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "second", "C", 0, 8));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 99, 100, 141, 142 })));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "third", "C", 0, 8));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 99, 141, 142 })));
  ASSERT_NO_FATAL_FAILURE(query_val(r, ar, "end", "C", 0, 8));
  ASSERT_EQ(*r, shared_ptr<value_set>(new vs_finite({ 100, 141, 142 })));
}


/*
 * <= including test9.as
 */
