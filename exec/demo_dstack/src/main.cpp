#include <summy/big_step/dectran.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/node/address_node.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <bjutil/binary/elf_provider.h>
#include <bjutil/printer.h>
#include <bjutil/gdsl_init.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <summy/analysis/domains/dstack.h>
#include <iosfwd>
#include <vector>
#include <map>
#include <tuple>
#include <iostream>
#include <fstream>
#include <functional>

#include <summy/analysis/fixpoint.h>
#include <summy/transformers/resolved_connector.h>

#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_finite.h>
#include <summy/value_set/vs_open.h>
#include <summy/value_set/vs_top.h>

#include <cstdio>

using analysis::fixpoint;
using cfg::address_node;
using cfg::edge;

using namespace gdsl::rreil;
using namespace std;
using namespace analysis;
using namespace summy;

int main(int argc, char **argv) {
  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

//  auto compiled = std::vector<uint8_t> { 0x48, 0xc7, 0xc0, 0x63, 0x0, 0x0, 0x0, 0xb8, 0x14, 0x0, 0x0, 0x0 };
//  g.set_code(compiled.data(), compiled.size(), 0);
//    dectran dt(g, false);

//  auto buffer = example(g, 0);
//  bj_gdsl bjg = gdsl_init_binfile(&f, "example.bin", 0);
//  bj_gdsl bjg = gdsl_init_immediate(&f, 0x00000000, 0);
  bj_gdsl bjg = gdsl_init_elf(&f, "a.out", ".text", "main", (size_t)1000);
  dectran dt(*bjg.gdsl, false);

  dt.transduce();
  dt.register_();

  try {

  auto &cfg = dt.get_cfg();
  cfg.commit_updates();

  dstack ds(&cfg);
  fixpoint fp(&ds);

  fp.iterate();

  cout << "++++++++++" << endl;
  ds.put(cout);
  cout << "++++++++++" << endl;

  ofstream dot_fs;
  dot_fs.open("output.dot", ios::out);
  cfg.dot(dot_fs);
  dot_fs.close();

  } catch(string &s) {
    cout << s << endl;
  }

//  g.set_code(NULL, 0, 0);
//  free(buffer);
  return 0;
}
