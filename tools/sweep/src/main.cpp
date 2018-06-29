#include <bjutil/binary/elf_provider.h>
#include <bjutil/gdsl_init.h>
#include <bjutil/printer.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/gdsl.h>
#include <fstream>
#include <functional>
#include <iosfwd>
#include <iostream>
#include <limits.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/node/address_node.h>
#include <tuple>
#include <vector>

#include <algorithm>
#include <bjutil/sort.h>
#include <optional>
#include <summy/analysis/fcollect/fcollect.h>
#include <summy/analysis/fixpoint.h>
#include <summy/analysis/static_memory.h>
#include <summy/big_step/analysis_dectran.h>
#include <summy/big_step/sweep.h>
#include <summy/cfg/jd_manager.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/statistics.h>
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

std::set<size_t> run_fcollect(gdsl::gdsl &g, bool blockwise_optimized) {
  cout << "\033[1;31m*** Starting the 'fcollect' analysis...\033[0m" << endl;
  sweep sweep(g, blockwise_optimized, true);
  sweep.transduce();
  analysis::fcollect::fcollect fc(&sweep.get_cfg());
  cfg::jd_manager jd_man_fc(&sweep.get_cfg());
  fixpoint fp_collect(&fc, jd_man_fc, true);
  fp_collect.iterate();

  size_t loc_sweep = loc_statistics(sweep.get_cfg()).get_loc();
  cout << "Loc (sweep): " << loc_sweep << endl;
  cout << "Decode iterations (sweep): " << sweep.get_decode_iterations() << endl;

  //    for(size_t address : fc.result().result)
  //      cout << hex << address << dec << endl;
  set<size_t> fstarts = fc.result().result;

  condition_statistics_data_t c_stats = condition_statistics(sweep.get_cfg()).get_stats();
  cout << "Total conditions: " << c_stats.total_conditions << endl;
  cout << "Comparison conditions: " << c_stats.cmp_conditions << " ("
       << (100.0 * c_stats.cmp_conditions / (float)c_stats.total_conditions) << "%)" << endl;

  return fstarts;
}

int main(int argc, char **argv) {
  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  elf_provider elfp = elf_provider(argv[1]);
  binary_provider::entry_t section;
  bool success;
  tie(success, section) = elfp.section(".text");
  if(!success) throw string("Invalid section .text");

  unsigned char *buffer = (unsigned char *)malloc(section.size);
  memcpy(buffer, elfp.get_data().data + section.offset, section.size);

  //  size_t size = (function.offset - section.offset) + function.size + 1000;
  //  if(size > section.size) size = section.size;

  g.set_code(buffer, section.size, section.address);

  bool blockwise_optimized = true;
  bool ref_management = true;
  bool tabulate = true;
  // std::optional<std::set<std::string>> filter_functions = nullopt;
  std::optional<std::set<std::string>> filter_functions = std::set{std::string("#fcollect_50")};

  try {
    std::set<size_t> fstarts = run_fcollect(g, blockwise_optimized);

    cout << "*** Collecting functions from ELF data..." << endl;
    auto functions = elfp.functions();

    function_map_t function_map;
    {
      size_t i = 0;
      for(size_t addr : fstarts)
        function_map[addr] = std::string("#fcollect_") + std::to_string(i++);
    }
    for(const auto &[name, e] : functions)
      function_map[e.address] = name;


    analysis_dectran dt(g, blockwise_optimized, false, function_map);
    dt.register_();

    for(const auto &[address, name] : function_map) {
      if(filter_functions && filter_functions->find(name) == filter_functions->end()) continue;
      // if(name != "sweep") continue;
      cout << "Adding function @" << hex << address << dec << " (" << name << ")" << endl;
      try {
        dt.transduce_function(address, name);
        auto &cfg = dt.get_cfg();
        cfg.commit_updates();
      } catch(string &s) {
        cout << "\033[1;31m-> Exception while adding :-/\033[0m" << endl;
      }
    }
    if(function_map.size() == 0) {
      size_t entry_address = elfp.entry_address();
      dt.transduce_function(entry_address, string("@@_entry"));
      auto &cfg = dt.get_cfg();
      cfg.commit_updates();
    }

    std::cout << "+++ All functions:" << std::endl;
    for(const auto &[address, name] : function_map)
      cout << "0x400000 + " << address << ", ";
    cout << std::endl;

    // cout << "** Adding Additionally collected functions..." << endl;
    // for(size_t address : fstarts) {
    //   //      cout << hex << address << dec << endl;
    //   try {
    //     dt.transduce_function(address);
    //     auto &cfg = dt.get_cfg();
    //     cfg.commit_updates();
    //   } catch(string &s) {
    //     //        cout << "\t Unable to seek!" << endl;
    //   }
    // }

    auto &cfg = dt.get_cfg();

    //    ofstream dot_noa_fs;
    //    dot_noa_fs.open("output_noa.dot", ios::out);
    //    cfg.dot(dot_noa_fs);
    //    dot_noa_fs.close();

    //    return 0;

    shared_ptr<static_memory> se = make_shared<static_elf>(&elfp);
    summary_dstack ds(&cfg, se, false, dt.get_f_heads(), tabulate);
    cfg::jd_manager jd_man(&cfg);
    fixpoint fp(&ds, jd_man, ref_management);

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
      if(n.get_id() == 604 || n.get_id() == 436 || n.get_id() == 618) {
        // out << n.get_id() << " [label=\"" << n.get_id() << "\n" << *ds.get(n.get_id()) << "\"]";
        out << n.get_id() << " [label=\"" << n.get_id() << "\n";
        for(auto ctx_mapping : ds.get_ctxful(n.get_id()))
          out << "CTX: " << ctx_mapping.first << "\t" << *ctx_mapping.second << endl;


        out << "\"]";
      } else
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

    size_t loc = loc_statistics(dt.get_cfg()).get_loc();
    cout << "Loc: " << loc << endl;
    cout << "Decode iterations: " << dt.get_decode_iterations() << endl;

    branch_statistics bs(g, ds, jd_man);
    auto b_stats = bs.get_stats();
    size_t total_indirect = b_stats.calls_total_indirect + b_stats.jmps_total_indirect;
    size_t with_targets = b_stats.calls_with_targets + b_stats.jmps_with_targets;
    cout << "Total indirect branches: " << total_indirect << endl;
    cout << "Indirect branches with targets: " << with_targets << " ("
         << (100.0 * with_targets / (float)total_indirect) << "%)" << endl;

    cout << "Total indirect jmps: " << b_stats.jmps_total_indirect << endl;
    cout << "Indirect jmps with targets: " << b_stats.jmps_with_targets << " ("
         << (100.0 * b_stats.jmps_with_targets / (float)b_stats.jmps_total_indirect) << "%)"
         << endl;

    cout << "Total indirect calls: " << b_stats.calls_total_indirect << endl;
    cout << "Indirect calls with targets: " << b_stats.calls_with_targets << " ("
         << (100.0 * b_stats.calls_with_targets / (float)b_stats.calls_total_indirect) << "%)"
         << endl;

    dt.print_decoding_holes();

    auto const &pointer_props = ds.get_pointer_props();
    for(auto const &pp : pointer_props) {
      cout << "PP for address 0x" << std::hex << pp.first << std::dec << ":" << std::endl;
      for(auto const &fr : pp.second) {
        cout << "  -> Field requirement " << std::endl;
        for(auto const &ptr : fr.second)
          cout << "    -> Propagated address 0x" << std::hex << ptr << std::dec << std::endl;
      }
    }

    auto const &hb_counts = ds.get_hb_counts();
    size_t requests = 0;
    size_t hbs = 0;
    for(auto const &head_mapping : hb_counts) {
      for(auto const &hb_mapping : head_mapping.second) {
        requests++;
        hbs += hb_mapping.second.size();
      }
    }
    cout << "(only valid if accumulating) Total requests: " << requests
         << ", instantiations: " << hbs << endl;

    auto const &ds_functions = ds.get_function_desc_map();
    size_t max_entries = 0;
    size_t entries_sum = 0;
    size_t field_requests = 0;
    for(auto const &[address, fd] : ds_functions) {
      const auto &state_map = ds.get_ctxful(fd.head_id);
      size_t table_entries = state_map.size();
      if(table_entries > max_entries) max_entries = table_entries;
      entries_sum += table_entries;
      field_requests += fd.field_reqs.size();
    }
    cout << "Maximum table entries: " << max_entries << endl;
    cout << "Average table entries: " << (entries_sum / (double)ds_functions.size()) << endl;
    cout << "Total number of functions: " << ds_functions.size() << endl;
    cout << "Total table entries: " << entries_sum << endl;
    cout << "Total number of field requests: " << field_requests << endl;

    const auto &context_uses = ds.get_context_uses();
    size_t nonzero_contexts_at_head = 0;
    size_t nonzero_users = 0;
    size_t zero_contexts_at_head = 0;
    size_t zero_users = 0;
    for(auto const &[head_node, context_user_map] : context_uses) {
      // cout << "++- head node: " << head_node << endl;
      // auto state_map = ds.get_ctxful(head_node);
      nonzero_contexts_at_head += context_user_map.size();
      bool has_zero = false;
      for(const auto &[context, users] : context_user_map) {
        // cout << " ~~~ context: " << context << endl;
        // cout << " ~~~ users: " << users.size() << endl;
        if(context == 0) {
          zero_users += users.size();
          has_zero = true;
        } else
          nonzero_users += users.size();
      }
      if(has_zero) {
        nonzero_contexts_at_head--;
        zero_contexts_at_head++;
      }
    }
    cout << "Non-zero contexts at head nodes: " << nonzero_contexts_at_head
         << " (zero: " << zero_contexts_at_head << ")" << endl;
    cout << "Non-zero users of contexts: " << nonzero_users << " (zero: " << zero_users << ")"
         << endl;

    auto const &path_construction_errors = ds.get_path_construction_errors();
    size_t path_errors_total = 0;
    for(auto const &[node, context_path_errors] : path_construction_errors)
      for(auto const &[context, path_errors] : context_path_errors)
        path_errors_total += path_errors;
    cout << "Path construction errors: " << path_errors_total << endl;

    auto const &unique_hbs = ds.get_unique_hbs();
    size_t zero_hbs = 0;
    size_t one_hb = 0;
    size_t multiple_hbs = 0;
    for(auto const &uh : unique_hbs) {
      if(uh.second == 0)
        zero_hbs++;
      else if(uh.second == 1)
        one_hb++;
      else
        multiple_hbs++;
    }
    cout << "Zero HBs: " << zero_hbs << ", one HB: " << one_hb << ", multiple HBs: " << multiple_hbs
         << endl;

    cout << "Hot addresses:" << endl;
    fp.print_hot_addresses();
  } catch(string &s) {
    cout << "Exception: " << s << endl;
    exit(1);
  }

  g.set_code(NULL, 0, 0);
  free(buffer);

  return 0;
}
