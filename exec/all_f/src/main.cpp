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
#include <summy/analysis/fcollect/fcollect.h>
#include <summy/big_step/analysis_dectran.h>
#include <summy/big_step/sweep.h>
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

//  binary_provider::entry_t function;
//  tie(ignore, function) = elfp.symbol("main");

  unsigned char *buffer = (unsigned char *)malloc(section.size);
  memcpy(buffer, elfp.get_data().data + section.offset, section.size);

//  size_t size = (function.offset - section.offset) + function.size + 1000;
//  if(size > section.size) size = section.size;

  g.set_code(buffer, section.size, section.address);

  try {
    cout << "\033[1;31m*** Starting the 'fcollect' analysis...\033[0m" << endl;
    sweep sweep(g, false, false);
    sweep.transduce();
    analysis::fcollect::fcollect fc(&sweep.get_cfg());
    cfg::jd_manager jd_man_fc(&sweep.get_cfg());
    fixpoint fp_collect(&fc, jd_man_fc);
    fp_collect.iterate();

//    for(size_t address : fc.result().result)
//      cout << hex << address << dec << endl;
    set<size_t> fstarts = fc.result().result;

    //  bj_gdsl bjg = gdsl_init_elf(&f, argv[1], ".text", "main", (size_t)1000);
    analysis_dectran dt(g, true, false);
    dt.register_();

//    int n = 0;

    cout << "*** Function from ELF data..." << endl;
    auto functions = elfp.functions();
    for(auto f : functions) {
      binary_provider::entry_t e;
      string name;
      tie(name, e) = f;
//      if(name != "p_slash_66_slash_f3Action2" && name != "p_slash_66_slash_f3_slash_f2Action8" && name != "with_f3") {
//        continue;
//      }
//      if(name != "_slash_vex_slash_0f_slash_vexv")
//        continue;
//      if(name != "sem_movsAction1")
//        continue;
//      if(name != "rreil_convert_sem_stmt")
//        continue;
      cout << hex << e.address << dec << " (" << name << ")" << endl;
      try {
        fstarts.erase(e.address);
        dt.transduce_function(e.address, name);
        auto &cfg = dt.get_cfg();
        cfg.commit_updates();
      } catch(string &s) {
//        cout << "\t Unable to seek!" << endl;
      }
    }
    if(functions.size() == 0) {
      size_t entry_address = elfp.entry_address();
      fstarts.erase(entry_address);
      dt.transduce_function(entry_address, string("@@_entry"));
      auto &cfg = dt.get_cfg();
      cfg.commit_updates();
    }

    cout << "*** Additionally collected functions..." << endl;
    for(size_t address : fstarts) {
//      cout << hex << address << dec << endl;
      try {
        dt.transduce_function(address);
        auto &cfg = dt.get_cfg();
        cfg.commit_updates();
      } catch(string &s) {
//        cout << "\t Unable to seek!" << endl;
      }
    }

    auto &cfg = dt.get_cfg();

//    ofstream dot_noa_fs;
//    dot_noa_fs.open("output_noa.dot", ios::out);
//    cfg.dot(dot_noa_fs);
//    dot_noa_fs.close();

//    return 0;


    shared_ptr<static_memory> se = make_shared<static_elf>(&elfp);
    summary_dstack ds(&cfg, se, false, dt.get_f_heads());
    cfg::jd_manager jd_man(&cfg);
    fixpoint fp(&ds, jd_man);

    cout << "\033[1;31mStarting main analysis.\033[0m" << endl;

    fp.iterate();

    cout << "\033[1;31mEnd of main analysis.\033[0m" << endl;
    fp.print_distribution_total();
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
      if(n.get_id() == 36 || n.get_id() == 39 || n.get_id() == 40 || n.get_id() == 47 || n.get_id() == 70)
        out << n.get_id() << " [label=\"" << n.get_id() << "\n" << *ds.get(n.get_id()) << "\"]";
//      out << n.get_id() << " [label=\"" << *jd_man.address_of(n.get_id()) << "\"]";
      else
        n.dot(out);
    });
    dot_fs.close();

    unique_ptr<cfg::cfg> machine_cfg = cfg.machine_cfg(false);
    ofstream dot_machine_fs;
    dot_machine_fs.open("output_machine.dot", ios::out);
    machine_cfg->dot(dot_machine_fs);
    dot_machine_fs.close();

    printf("Section size: %zu\n", section.size);
    printf("Decoded bytes: %lld\n", dt.bytes_decoded());
    printf("Analyzed addresses: %zu\n", fp.analyzed_addresses());
    printf("Decoded start addresses: %lld\n", dt.start_addresses_decoded());
    dt.print_decoding_holes();
  } catch(string &s) {
    cout << "Exception: " << s << endl;
    exit(1);
  }

  g.set_code(NULL, 0, 0);
  free(buffer);

  return 0;
}
