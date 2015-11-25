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
//#include <summy/analysis/domains/dstack.h>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/analysis/domains/numeric/vsd_state.h>
#include <iosfwd>
#include <vector>
#include <map>
#include <tuple>
#include <iostream>
#include <fstream>
#include <functional>

#include <summy/analysis/fixpoint.h>
#include <summy/analysis/static_memory.h>
#include <summy/cfg/jd_manager.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/transformers/resolved_connector.h>
#include <bjutil/sort.h>

#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_finite.h>
#include <summy/value_set/vs_open.h>
#include <summy/value_set/vs_top.h>
#include <algorithm>

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
  //  int a, b, c;
  //  tie(a, b, c) = tsort(9, 2, 10);
  //
  //  cout << a << endl;
  //  cout << b << endl;
  //  cout << c << endl;
  //
  ////  auto v = foo(9, 2, 10);
  ////  for(auto x : v)
  ////    cout << x << endl;
  //
  //  exit(0);

  //  set<int, function<bool (int, int)>> s([&](int a, int b) {
  //    return a > b;
  //  });
  //
  //  s.insert(10);
  //  s.insert(20);
  //
  //  for(int x : s)
  //    cout << x << endl;
  //
  //  exit(0);


  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  elf_provider elfp = elf_provider(argv[1]);
  binary_provider::entry_t section;
  bool success;
  tie(success, section) = elfp.section(".text");
  if(!success) throw string("Invalid section .text");

  binary_provider::entry_t function;
  tie(ignore, function) = elfp.symbol("main");

  unsigned char *buffer = (unsigned char *)malloc(section.size);
  memcpy(buffer, elfp.get_data().data + section.offset, section.size);

  size_t size = (function.offset - section.offset) + function.size + 1000;
  if(size > section.size) size = section.size;

  g.set_code(buffer, size, section.address);
  if(g.seek(function.address)) {
    throw string("Unable to seek to given function_name");
  }

  try {
    //  bj_gdsl bjg = gdsl_init_elf(&f, argv[1], ".text", "main", (size_t)1000);
    dectran dt(g, false);

    dt.transduce();
    dt.register_();

    auto &cfg = dt.get_cfg();
    cfg.commit_updates();

    shared_ptr<static_memory> se = make_shared<static_elf>(&elfp);
    summary_dstack ds(&cfg, se);
    jd_manager jd_man(&cfg);
    fixpoint fp(&ds, jd_man);

    fp.iterate();

    ofstream dot_noa_fs;
    dot_noa_fs.open("output_noa.dot", ios::out);
    cfg.dot(dot_noa_fs);
    dot_noa_fs.close();

    //  cout << "++++++++++" << endl;
    //  ds.put(cout);
    //  cout << "++++++++++" << endl;

    ofstream dot_fs;
    dot_fs.open("output.dot", ios::out);
    cfg.dot(dot_fs, [&](cfg::node &n, ostream &out) {
      if(n.get_id() == 120 || n.get_id() == 59 || n.get_id() == 54 || n.get_id() == 36)
        out << n.get_id() << " [label=\"" << n.get_id() << "\n" << *ds.get(n.get_id()) << "\"]";
      else
        n.dot(out);
    });
    dot_fs.close();

  } catch(string s) {
    cout << "Exception: " << s << endl;
    exit(1);
  }

  g.set_code(NULL, 0, 0);
  free(buffer);
  return 0;
}
