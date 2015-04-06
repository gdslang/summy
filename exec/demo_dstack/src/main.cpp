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
#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/domains/dstack.h>
#include <summy/analysis/domains/numeric/vsd_state.h>
#include <iosfwd>
#include <vector>
#include <map>
#include <tuple>
#include <iostream>
#include <fstream>
#include <functional>

#include <summy/analysis/fixpoint.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/transformers/resolved_connector.h>

#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_finite.h>
#include <summy/value_set/vs_open.h>
#include <summy/value_set/vs_top.h>

#include <cstdio>
#include <memory>

using analysis::fixpoint;
using analysis::value_sets::vsd_state;
using cfg::address_node;
using cfg::edge;
using summy::rreil::numeric_id;

using namespace gdsl::rreil;
using namespace std;
using namespace analysis;
using namespace summy;
using namespace analysis::api;

int main(int argc, char **argv) {
//  int foo = 42;
//  FILE *fi = fmemopen(&foo, 4, "r");
//  int fd = fileno(fi);
//
//  int abc;
////  fread(&abc, 4, 1, fi);
//  int r = read(fd, &abc, 4);
//  cout << r << " /// " << abc << endl;
//
//  exit(0);
  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

//  auto compiled = std::vector<uint8_t> { 0x48, 0xc7, 0xc0, 0x63, 0x0, 0x0, 0x0, 0xb8, 0x14, 0x0, 0x0, 0x0 };
//  g.set_code(compiled.data(), compiled.size(), 0);
//    dectran dt(g, false);

//  auto buffer = example(g, 0);
//  bj_gdsl bjg = gdsl_init_binfile(&f, "example.bin", 0);
//  bj_gdsl bjg = gdsl_init_immediate(&f, 0x00000000, 0);


//  vsd_state vsds;
////  num_expr_cmp *cmp = new num_expr_cmp(new num_linear_term(new num_var(numeric_id::generate()), new num_linear_vs(vs_finite::single(9))), EQ);
//
////  vs_shared_t vs = make_shared<vs_finite>(vs_finite::elements_t {-3, 0, 5});
////  vs_shared_t vs1 = make_shared<vs_finite>(vs_finite::elements_t { 1, 10});
////  vs_shared_t vs2 = make_shared<vs_finite>(vs_finite::elements_t {-2, -9});
//  vs_shared_t vs = make_shared<vs_open>(UPWARD,0);
//  num_expr *ass_exp = new num_expr_lin(new num_linear_vs(vs));
////  num_expr *ass_exp2 = new num_expr_lin(new num_linear_vs(vs2));
//  num_var *var = new num_var(numeric_id::generate());
////  num_var *var2 = new num_var(numeric_id::generate());
//  vsds.assign(var, ass_exp);
////  vsds.assign(var2, ass_exp2);
//
//  //v <= {3, -5}
//  //v + {-3, 5} <= 0;
//  // [-ue, 3]
//
//  //{1, 5} + {-2, -10} <= 0
//  //{1, 5} <= - {-2, -10} - [0, ue]
//  //{1, 5} <= {2, 10} - [0, ue]
//  //{1, 5} <= {2, 10} + [-ue, 0]
//  //{1, 5} <= [-ue, 10]
//
////  num_expr_cmp *cmp = new num_expr_cmp(new num_linear_term(new num_var(numeric_id::generate()), new num_linear_vs(vs)), LE);
//  num_expr_cmp *cmp = new num_expr_cmp(new num_linear_term(var), NEQ);
//  vsds.assume(cmp);
//  cout << vsds << endl;
//
//  exit(0);

  try {
  bj_gdsl bjg = gdsl_init_elf(&f, argv[1], ".text", "main", (size_t)1000);
  dectran dt(*bjg.gdsl, false);

  dt.transduce();
  dt.register_();

  auto &cfg = dt.get_cfg();
  cfg.commit_updates();

  dstack ds(&cfg);
  fixpoint fp(&ds);

  fp.iterate();


//  cout << "++++++++++" << endl;
//  ds.put(cout);
//  cout << "++++++++++" << endl;

  ofstream dot_fs;
  dot_fs.open("output.dot", ios::out);
  cfg.dot(dot_fs, [&](cfg::node &n, ostream &out) {
    out << n.get_id() << " [label=\"" << n.get_id() << "\n" << *ds.get(n.get_id()) << "\"]";
  });
//  cfg.dot(dot_fs);
  dot_fs.close();

  } catch(string *s) {
    cout << "Exception: " << *s << endl;
    exit(1);
  }

//  g.set_code(NULL, 0, 0);
//  free(buffer);
  return 0;
}
