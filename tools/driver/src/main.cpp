#include <algorithm>
#include <bjutil/binary/elf_provider.h>
#include <bjutil/gdsl_init.h>
#include <bjutil/printer.h>
#include <bjutil/sort.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/gdsl.h>
#include <fmt/printf.h>
#include <fstream>
#include <functional>
#include <limits.h>
#include <map>
#include <memory>
#include <optional>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/analysis/fcollect/fcollect.h>
#include <summy/analysis/fixpoint.h>
#include <summy/analysis/static_memory.h>
#include <summy/big_step/analysis_dectran.h>
#include <summy/big_step/sweep.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/jd_manager.h>
#include <summy/cfg/node/address_node.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/statistics.h>
#include <summy/transformers/resolved_connector.h>
#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_finite.h>
#include <summy/value_set/vs_open.h>
#include <summy/value_set/vs_top.h>
#include <tuple>
#include <vector>

using analysis::fixpoint;
using analysis::value_sets::vsd_state;
using cfg::address_node;
using cfg::edge;
using summy::rreil::numeric_id;

using namespace gdsl::rreil;
using namespace analysis;
using namespace summy;
using namespace analysis::api;

std::set<size_t> run_fcollect(gdsl::gdsl &g, bool blockwise_optimized) {
  fmt::printf("\033[1;31m*** Starting the 'fcollect' analysis...\033[0m\n");
  sweep sweep(g, blockwise_optimized, true);
  sweep.transduce();
  analysis::fcollect::fcollect fc(&sweep.get_cfg());
  cfg::jd_manager jd_man_fc(&sweep.get_cfg());
  fixpoint fp_collect(&fc, jd_man_fc, true);
  fp_collect.iterate();

  size_t loc_sweep = loc_statistics(sweep.get_cfg()).get_loc();
  fmt::printf("Loc (sweep): %d\n", loc_sweep);
  fmt::printf("Decode iterations (sweep): %d\n", sweep.get_decode_iterations());

  //    for(size_t address : fc.result().result)
  //      fmt::printf( hex << address << dec );
  std::set<size_t> fstarts = fc.result().result;

  condition_statistics_data_t c_stats = condition_statistics(sweep.get_cfg()).get_stats();
  fmt::printf("Total conditions: %d\n", c_stats.total_conditions);
  fmt::printf("Comparison conditions: %d (%f)\n", c_stats.cmp_conditions,
    100.0 * c_stats.cmp_conditions / (float)c_stats.total_conditions);

  return fstarts;
}

int main(int argc, char **argv) {
  bool blockwise_optimized = true;
  bool ref_management = true;
  bool tabulate = true;
  std::optional<std::set<std::string>> filter_functions = std::nullopt;
  // std::optional<std::set<std::string>> filter_functions = std::set{std::string("XYZ")};
  std::optional<std::set<size_t>> filter_nodes = std::nullopt;
  // std::optional<std::set<size_t>> filter_nodes = std::set<size_t>{1, 2, 3};

  char *path = nullptr;
  for(int i = 1; i < argc; i++) {
    auto insert = [&](auto &opt_set, auto value) {
      if(!opt_set) opt_set = std::set<typeof(value)>();
      opt_set->insert(value);
    };
    std::string arg = argv[i];
    const string func_str("--func");
    const string node_str("--node=");
    if(!arg.find("--noopt"))
      blockwise_optimized = false;
    else if(!arg.find("--noref"))
      ref_management = false;
    else if(!arg.find("--notab"))
      tabulate = false;
    else if(!arg.find(func_str)) {
      string func = arg.substr(func_str.length());
      insert(filter_functions, std::move(func));
    } else if(!arg.find(node_str)) {
      size_t node = (size_t)std::atoll(arg.substr(node_str.length()).c_str());
      insert(filter_nodes, node);
    } else if(!arg.find("--")) {
      fmt::printf("Invalid argument: %s\n", arg);
      return 2;
    } else
      path = argv[i];
  }
  if(path == nullptr) {
    fmt::printf("Please pass the path to the executable.\n");
    return 1;
  }

  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  elf_provider elfp = elf_provider(path);
  binary_provider::entry_t section;
  bool success;
  std::tie(success, section) = elfp.section(".text");
  if(!success) throw string("Invalid section .text");

  unsigned char *buffer = (unsigned char *)malloc(section.size);
  memcpy(buffer, elfp.get_data().data + section.offset, section.size);

  //  size_t size = (function.offset - section.offset) + function.size + 1000;
  //  if(size > section.size) size = section.size;

  g.set_code(buffer, section.size, section.address);

  try {
    std::set<size_t> fstarts = run_fcollect(g, blockwise_optimized);

    fmt::printf("*** Collecting functions from ELF data...\n");
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
      fmt::printf("Adding function @%x (%s)\n", address, name);
      try {
        dt.transduce_function(address, name);
        auto &cfg = dt.get_cfg();
        cfg.commit_updates();
      } catch(string &s) {
        fmt::printf("\033[1;31m-> Exception while adding :-/\033[0m\n");
      }
    }
    if(function_map.size() == 0) {
      size_t entry_address = elfp.entry_address();
      dt.transduce_function(entry_address, string("@@_entry"));
      auto &cfg = dt.get_cfg();
      cfg.commit_updates();
    }

    fmt::printf("+++ All functions:");
    for(const auto &[address, name] : function_map)
      fmt::printf("0x400000 + %d, ", address);
    fmt::printf("\n");

    auto &cfg = dt.get_cfg();

    shared_ptr<static_memory> se = std::make_shared<static_elf>(&elfp);
    summary_dstack ds(&cfg, se, false, dt.get_f_heads(), tabulate);
    cfg::jd_manager jd_man(&cfg);
    fixpoint fp(&ds, jd_man, ref_management);

    fmt::printf("\033[1;31mStarting main analysis.\033[0m\n");

    fp.iterate();

    fmt::printf("\033[1;31mEnd of main analysis.\033[0m\n");
    fp.print_distribution_total();
    fmt::printf("Max its: %d\n", fp.max_iter());

    // std::ofstream dot_noa_fs;
    // dot_noa_fs.open("output_noa.dot", std::ios::out);
    // cfg.dot(dot_noa_fs);
    // dot_noa_fs.close();

    std::ofstream dot_fs;
    dot_fs.open("cfg.dot", std::ios::out);
    cfg.dot(dot_fs, [&](cfg::node &n, std::ostream &out) {
      if(filter_nodes && filter_nodes->find(n.get_id()) != filter_nodes->end()) {
        fmt::fprintf(out, "%d [label=\"%d\n", n.get_id(), n.get_id());
        for(auto ctx_mapping : ds.get_ctxful(n.get_id()))
          fmt::fprintf(out, "CTX: %d\t%s\n", ctx_mapping.first, *ctx_mapping.second);
        fmt::fprintf(out, "\"]");
      } else
        n.dot(out);
    });
    dot_fs.close();

    std::unique_ptr<cfg::cfg> machine_cfg = cfg.machine_cfg(false);
    std::ofstream dot_machine_fs;
    dot_machine_fs.open("cfg_machine.dot", std::ios::out);
    machine_cfg->dot(dot_machine_fs);
    dot_machine_fs.close();

    fmt::printf("Section size: %d\n", section.size);
    fmt::printf("Decoded bytes: %d\n", dt.bytes_decoded());
    fmt::printf("Analyzed addresses: %d\n", fp.analyzed_addresses());
    fmt::printf("Decoded start addresses: %d\n", dt.start_addresses_decoded());

    size_t loc = loc_statistics(dt.get_cfg()).get_loc();
    fmt::printf("Loc: %d\n", loc);
    fmt::printf("Decode iterations: %d\n", dt.get_decode_iterations());

    branch_statistics bs(g, ds, jd_man);
    auto b_stats = bs.get_stats();
    size_t total_indirect = b_stats.calls_total_indirect + b_stats.jmps_total_indirect;
    size_t with_targets = b_stats.calls_with_targets + b_stats.jmps_with_targets;
    fmt::printf("Total indirect branches: %d\n", total_indirect);
    fmt::printf("Indirect branches with targets: %d (%f%%)\n", with_targets,
      (100.0 * with_targets / (float)total_indirect));

    fmt::printf("Total indirect jmps: %d\n", b_stats.jmps_total_indirect);
    fmt::printf("Indirect jmps with targets: %d (%f%%)\n", b_stats.jmps_with_targets,
      100.0 * b_stats.jmps_with_targets / (float)b_stats.jmps_total_indirect);

    fmt::printf("Total indirect calls: %d\n", b_stats.calls_total_indirect);
    fmt::printf("Indirect calls with targets: %d (%f%%)\n", b_stats.calls_with_targets,
      100.0 * b_stats.calls_with_targets / (float)b_stats.calls_total_indirect);

    dt.print_decoding_holes();

    auto const &pointer_props = ds.get_pointer_props();
    for(auto const &pp : pointer_props) {
      fmt::printf("PP for address 0x%x:\n", pp.first);
      for(auto const &fr : pp.second) {
        fmt::printf("  -> Field requirement \n");
        for(auto const &ptr : fr.second)
          fmt::printf("    -> Propagated address 0x%x\n", ptr);
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
    fmt::printf(
      "(only valid if accumulating) Total requests: %d, instantiations: %d\n", requests, hbs);

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
    fmt::printf("Maximum table entries: %d\n", max_entries);
    fmt::printf("Average table entries: %f\n", entries_sum / (double)ds_functions.size());
    fmt::printf("Total number of functions: %d\n", ds_functions.size());
    fmt::printf("Total table entries: %d\n", entries_sum);
    fmt::printf("Total number of field requests: %d\n", field_requests);

    const auto &context_uses = ds.get_context_uses();
    size_t nonzero_contexts_at_head = 0;
    size_t nonzero_users = 0;
    size_t zero_contexts_at_head = 0;
    size_t zero_users = 0;
    for(auto const &[head_node, context_user_map] : context_uses) {
      // fmt::printf( "++- head node: " , head_node );
      // auto state_map = ds.get_ctxful(head_node);
      nonzero_contexts_at_head += context_user_map.size();
      bool has_zero = false;
      for(const auto &[context, users] : context_user_map) {
        // fmt::printf( " ~~~ context: " , context );
        // fmt::printf( " ~~~ users: " , users.size() );
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
    fmt::printf("Non-zero contexts at head nodes: %d (zero: %d)\n", nonzero_contexts_at_head,
      zero_contexts_at_head);
    fmt::printf("Non-zero users of contexts: %d (zero: %d)\n", nonzero_users, zero_users);

    auto const &path_construction_errors = ds.get_path_construction_errors();
    size_t path_errors_total = 0;
    for(auto const &[node, context_path_errors] : path_construction_errors)
      for(auto const &[context, path_errors] : context_path_errors)
        path_errors_total += path_errors;
    fmt::printf("Path construction errors: %d\n", path_errors_total);

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
    fmt::printf("Zero HBs: %d, one HB: %d, multiple HBs: %d\n", zero_hbs, one_hb, multiple_hbs);

    fmt::printf("Hot addresses:");
    fp.print_hot_addresses();
  } catch(string &s) {
    fmt::printf("Exception: %s", s);
    exit(1);
  }

  g.set_code(NULL, 0, 0);
  free(buffer);

  return 0;
}
