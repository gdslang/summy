#include <assert.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/node/address_node.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <bjutil/binary/elf_provider.h>
#include <bjutil/binary/special_functions.h>
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
#include <summy/big_step/analysis_dectran.h>
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
  bool success;

  binary_provider::entry_t dot_plt;
  tie(success, dot_plt) = elfp.section(".plt");
  if(!success) throw string("Invalid section .plt");

  binary_provider::entry_t dot_text;
  tie(success, dot_text) = elfp.section(".text");
  if(!success) throw string("Invalid section .text");

  assert(dot_plt.address + dot_plt.size == dot_text.address);
  size_t section_size = dot_plt.size + dot_text.size;
  size_t section_offset = dot_plt.offset;
  size_t section_address = dot_plt.address;

  binary_provider::entry_t function;
  tie(ignore, function) = elfp.symbol("main");

  unsigned char *buffer = (unsigned char *)malloc(section_size);
  memcpy(buffer, elfp.get_data().data + section_offset, section_size);

  size_t size = (function.offset - section_offset) + function.size + 1000;
  if(size > section_size) size = section_size;

  auto f_dyn = elfp.functions_dynamic();
  for(auto t : f_dyn) {
    string name;
    binary_provider::entry_t e;
    tie(name, e) = t;
    cout << name << "@" << e.address << endl;
  }

  g.set_code(buffer, size, section_address);
  if(g.seek(function.address)) {
    throw string("Unable to seek to given function_name");
  }

  try {
    //  bj_gdsl bjg = gdsl_init_elf(&f, argv[1], ".text", "main", (size_t)1000);
    analysis_dectran dt(g, false);

    dt.transduce();
    dt.register_();

    auto &cfg = dt.get_cfg();
    cfg.commit_updates();

    shared_ptr<static_memory> se = make_shared<static_elf>(&elfp);
    summary_dstack ds(&cfg, se, special_functions::from_elf_provider(elfp));
    cfg::jd_manager jd_man(&cfg);
    fixpoint fp(&ds, jd_man);
    cfg.register_observer(&fp);

    fp.iterate();
    cout << "Max its: " << fp.max_iter() << endl;

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
      if(n.get_id() == 84)
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
