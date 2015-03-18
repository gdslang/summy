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

//  auto buffer = example(g, 0);
//  bj_gdsl bjg = gdsl_init_binfile(&f, "example.bin", 0);
  bj_gdsl bjg = gdsl_init_elf(&f, "a.out", ".text", "main", (size_t)1000);
//  bj_gdsl bjg = gdsl_init_immediate(&f, 0x00000000, 0);

//  dectran dt(*bjg.gdsl, false);
  dectran dt(*bjg.gdsl, false);
  dt.transduce();
  dt.register_();

  auto &cfg = dt.get_cfg();


  ofstream dot_fs;
  dot_fs.open("output.dot", ios::out);
  cfg.dot(dot_fs);
  dot_fs.close();

//  g.set_code(NULL, 0, 0);
//  free(buffer);
  return 0;
}
