#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/block.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/instruction.h>
#include <cppgdsl/rreil/rreil.h>

#include <vector>
#include <map>
#include <tuple>

#include <summy/cfg/cfg.h>
#include <summy/cfg/node/node.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/bfs_iterator.h>
#include <iostream>
#include <fstream>

#include <summy/analysis/analysis.h>

#include <summy/transformers/decomposer.h>
#include <summy/transformers/goto_ip_adder.h>
#include <summy/transformers/ip_propagator.h>
#include <summy/transformers/trivial_connector.h>

#include <summy/binary/elf_provider.h>

#include <cppgdsl/rreil/copy_visitor.h>

#include <summy/analysis/reaching_defs/reaching_defs.h>
#include <summy/analysis/adaptive_rd/adaptive_rd.h>
#include <summy/analysis/reaching_defs/rd_elem.h>
#include <summy/analysis/fixpoint.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/big_step/dectran.h>
#include <summy/big_step/ssa.h>
#include <summy/cfg/node/address_node.h>
#include <summy/transformers/ssa/phi_inserter.h>
#include <summy/transformers/ssa/renamer.h>

using cfg::address_node;
using cfg::edge;

using namespace gdsl::rreil;
using namespace std;

cfg::translated_program_t elf(gdsl::gdsl &g) {
  elf_provider elfp = [&]() {
    try {
      return elf_provider("a.out");
    } catch(string &s) {
      cout << "Error initializing elf provider: " << s << endl;
      throw string("no elf() :/");
    }
  }();
//  binary_provider::bin_range_t range = elfp.bin_range();
  binary_provider::data_t data = elfp.get_data();
  binary_provider::entry_t e;
  tie(ignore, e) = elfp.entry("main");

//  uint32_t buffer = 0xfc75c085;
  g.set_code(data.data + e.offset, e.size, e.address);

  vector<tuple<uint64_t, vector<gdsl::rreil::statement*>*>> prog;
  while(g.get_ip() < e.address + e.size) {
    int_t ip = g.get_ip();

    /*
     * Todo: Let trivial_connector report missing targets, build new cfgs for these,
     * transform them, union them with original cfg
     */
//    gdsl::block b = g.decode_translate_block(gdsl::preservation::CONTEXT, LONG_MAX);
//    auto rreil = b.get_statements();
    gdsl::instruction insn = g.decode();
    printf("Instruction: %s\n", insn.to_string().c_str());
    printf("---------------------------------\n");
    auto rreil = insn.translate();

    g.reset_heap();

    printf("RReil (no transformations):\n");
    for(statement *s : *rreil)
      printf("%s\n", s->to_string().c_str());

    prog.push_back(make_tuple(ip, rreil));
  }

  return prog;
}

cfg::translated_program_t manual(gdsl::gdsl &g, uint64_t ip) {
//  uint32_t buffer = 0x00ab48f3;
  uint32_t buffer = 0x00e2d348;
  g.set_code((unsigned char*)&buffer, sizeof(buffer), ip);

  cfg::translated_program_t prog;
  for(size_t i = 0; i < 1; i++) {
    int_t ip = g.get_ip();

    gdsl::instruction insn = g.decode();
    printf("Instruction: %s\n", insn.to_string().c_str());
    printf("---------------------------------\n");
    auto rreil = insn.translate();

    g.reset_heap();

    printf("RReil (no transformations):\n");
    for(statement *s : *rreil)
      printf("%s\n", s->to_string().c_str());

    prog.push_back(make_tuple(ip, rreil));
  }

  return prog;
}

cfg::cfg *gen_cfg(gdsl::gdsl &g, uint64_t ip) {
  auto prog = manual(g, ip);

  cfg::cfg *cfg = new cfg::cfg();
  cfg->add_program(prog);

  for(auto t : prog) {
    vector<gdsl::rreil::statement*>* rreil;
    tie(ignore, rreil) = t;
    for(auto stmt : *rreil)
      delete stmt;
    delete rreil;
  }

  vector<transformer*> transformers;
  transformers.push_back(new decomposer(cfg));
  transformers.push_back(new goto_ip_adder(cfg));
  transformers.push_back(new ip_propagator(cfg));
  transformers.push_back(new trivial_connector(cfg));
  for(auto t : transformers) {
    t->transform();
    delete t;
  }

  return cfg;
}

int main(void) {
  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  elf_provider elfp = [&]() {
    try {
      return elf_provider("a.out");
    } catch(string &s) {
      cout << "Error initializing elf provider: " << s << endl;
      throw string("no elf() :/");
    }
  }();
//  binary_provider::bin_range_t range = elfp.bin_range();
  binary_provider::data_t data = elfp.get_data();
  binary_provider::entry_t e;
  tie(ignore, e) = elfp.entry("main");

//  uint32_t buffer = 0xfc75c085;
  g.set_code(data.data + e.offset, e.size, e.address);

  dectran dt(g);
  dt.transduce_and_register();

  auto &cfg = dt.get_cfg();
  for(int i = 0; i < 10; i++)
    cfg.commit_updates();

//  cfg::cfg *cfg = gen_cfg(g, 0);
//  cfg::cfg *cfg2 = gen_cfg(g, 3);
//  cfg->merge(*cfg2, 49, 0);

//  auto foo = cfg.out_edges(178)->at(179);
//  cfg.erase_edge(178, 179);
//  auto ani = cfg.create_node([&](size_t id) {
//    return new address_node(id, 7777);
//  });
//  cfg.update_edge(ani, 179, new edge());
//
//  ssa ssa(cfg);
//  ssa.transduce();
//  cfg.clear_updates();
//
//  cfg.update_edge(178, ani, foo);
//  cfg.commit_updates();

//  ofstream dot_fsb;
//  dot_fsb.open("output_before.dot", ios::out);
//  cfg.dot(dot_fsb);
//  dot_fsb.close();
//
//  cfg.commit_updates();

  ofstream dot_fs;
  dot_fs.open("output.dot", ios::out);
  cfg.dot(dot_fs);
  dot_fs.close();

//  delete cfg;
//  delete cfg2;

  return 0;
}
