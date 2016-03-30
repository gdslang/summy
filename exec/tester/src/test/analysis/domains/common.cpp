/*
 * common.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: Julian Kranz
 */

#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/gdsl.h>
#include <gtest/gtest.h>
#include <summy/test/compile.h>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/analysis/fixpoint.h>
#include <summy/analysis/static_memory.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/jd_manager.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node_visitor.h>
#include <summy/test/analysis/domains/common.h>
#include <istream>
#include <memory>
#include <string>
#include <tuple>

using analysis::fixpoint;
using analysis::static_elf;
using analysis::static_memory;
using analysis::summary_dstack;
using cfg::address_node;
using cfg::bfs_iterator;
using cfg::jd_manager;
using cfg::node_visitor;
using std::basic_istream;
using std::make_shared;
using std::string;
using std::tie;
using std::ignore;

void state(_analysis_result &r, string program, language_t lang, bool gdsl_optimize, uint8_t compiler_opt) {
  SCOPED_TRACE("state()");

  switch(lang) {
    case ASSEMBLY: {
      string ___asm = program;
      r.elfp = asm_compile_elfp(___asm);
      break;
    }
    case C: {
      string filename = c_compile(program, compiler_opt);
      r.elfp = new elf_provider(filename.c_str());
      break;
    }
    case CPP: {
      string filename = cpp_compile(program, compiler_opt);
      r.elfp = new elf_provider(filename.c_str());
      break;
    }
  }

  //  binary_provider::entry_t e;
  //  tie(ignore, e) = elfp->entry("foo");
  //  cout << e.address << " " << e.offset << " " << e.size << endl;

  //  auto compiled = asm_compile(___asm);

  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  binary_provider::entry_t section;
  bool success;
  tie(success, section) = r.elfp->section(".text");
  if(!success) throw string("Invalid section .text");

  binary_provider::entry_t function;
  tie(ignore, function) = r.elfp->symbol("main");

  //  unsigned char *buffer = (unsigned char*)malloc(section.size);
  //  memcpy(buffer, elfp.get_data().data + section.offset, section.size);

  size_t size = (function.offset - section.offset) + function.size + 1000;
  if(size > section.size) size = section.size;

  g.set_code(r.elfp->get_data().data + section.offset, size, section.address);
  if(g.seek(function.address)) {
    throw string("Unable to seek to given function_name");
  }

  r.dt = new analysis_dectran(g, gdsl_optimize);
  r.dt->transduce();
  r.dt->register_();

  auto &cfg = r.dt->get_cfg();
  cfg.commit_updates();

  shared_ptr<static_memory> se = make_shared<static_elf>(r.elfp);
  r.ds_analyzed = new summary_dstack(&cfg, se, false);
  jd_manager jd_man(&cfg);
  fixpoint fp(r.ds_analyzed, jd_man);
  cfg.register_observer(&fp);
  fp.iterate();

  for(auto *node : cfg) {
    node_visitor nv;
    nv._([&](address_node *an) { r.addr_node_map[an->get_address()] = an->get_id(); });
    node->accept(nv);
  }

  r.max_it = fp.max_iter();
}

void state_asm(_analysis_result &r, string _asm, bool gdsl_optimize) {
  state(r, _asm, ASSEMBLY, gdsl_optimize, 1);
}

void state_c(_analysis_result &r, string c, bool gdsl_optimize) {
  state(r, c, C, gdsl_optimize, 1);
}

void state_cpp(_analysis_result &r, string c, bool gdsl_optimize) {
  state(r, c, CPP, gdsl_optimize, 1);
}
