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
#include <summy/cfg/node.h>
#include <summy/cfg/edge.h>
#include <summy/cfg/bfs_iterator.h>
#include <iostream>
#include <fstream>

#include <summy/transformers/decomposer.h>
#include <summy/transformers/goto_ip_adder.h>
#include <summy/transformers/ip_propagator.h>
#include <summy/transformers/trivial_connector.h>

#include <summy/binary/elf_provider.h>

#include <cppgdsl/rreil/copy_visitor.h>

using namespace gdsl::rreil;
using namespace std;

int main(void) {
  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  elf_provider elfp("a.out");
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

  cfg::cfg cfg(prog);

  for(auto t : prog) {
    vector<gdsl::rreil::statement*>* rreil;
    tie(ignore, rreil) = t;
    for(auto stmt : *rreil)
      delete stmt;
    delete rreil;
  }

  vector<transformer*> transformers;
  transformers.push_back(new decomposer(&cfg));
  transformers.push_back(new goto_ip_adder(&cfg));
  transformers.push_back(new ip_propagator(&cfg));
  transformers.push_back(new trivial_connector(&cfg));
  for(auto t : transformers) {
    t->transform();
    delete t;
  }

//  printf("RReil (after transformations):\n");
//  for(statement *s : *rreil)
//    printf("%s\n", s->to_string().c_str());

  ofstream dot_fs;
  dot_fs.open("output.dot", ios::out);
  cfg.dot(dot_fs);
  dot_fs.close();

  return 0;
}
