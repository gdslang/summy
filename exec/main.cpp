#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
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

unsigned char *elf(gdsl::gdsl &g) {
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

  unsigned char *buffer = (unsigned char*)malloc(e.offset);
  memcpy(buffer, data.data + e.offset, e.size);
  g.set_code(buffer, e.size, e.address);
  return buffer;
}

unsigned char *manual(gdsl::gdsl &g, uint64_t ip) {
//  uint32_t buffer = 0x00ab48f3;
  uint32_t *buffer = (uint32_t*)malloc(sizeof(uint32_t));
  *buffer = 0x00e2d348;
  g.set_code((unsigned char*)buffer, sizeof(*buffer), ip);
  return (unsigned char*)buffer;
}

int main(void) {
  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  auto buffer = elf(g);

  dectran dt(g, false);
  dt.transduce_and_register();

  auto &cfg = dt.get_cfg();
  cfg.commit_updates();

//  auto foo = cfg.out_edges(178)->at(179);
//  cfg.erase_edge(178, 179);
//  auto ani = cfg.create_node([&](size_t id) {
//    return new address_node(id, 7777);
//  });
//  cfg.update_edge(ani, 179, new edge());
//

  ssa ssa(cfg);
  ssa.transduce_and_register();
//  cfg.clear_updates();

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

  g.set_code(NULL, 0, 0);
  free(buffer);
  return 0;
}
