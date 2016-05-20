/*
 * statistics.cpp
 *
 *  Created on: May 20, 2016
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <cppgdsl/rreil/statement/branch.h>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/statistics.h>
#include <summy/tools/rreil_util.h>
#include <cppgdsl/rreil/statement/branch.h>

#include "cppgdsl/rreil/statement/statement_visitor.h"
#include <algorithm>
#include <istream>
#include <iterator>
#include <tuple>

using namespace std;

using analysis::node_targets_t;
using cfg::edge_visitor;
using cfg::stmt_edge;
using gdsl::rreil::branch;
using gdsl::rreil::statement_visitor;

branch_statistics::branch_statistics(gdsl::gdsl &gdsl, analysis::summary_dstack &sd, cfg::jd_manager &jd_manager)
    : gdsl(gdsl), sd(sd), jd_manager(jd_manager) {
  cfg::cfg *cfg = sd.get_cfg();

  node_targets_t const &node_targets = sd.get_targets();

  for(auto nt_it : node_targets) {
    size_t node_from = nt_it.first;
    cfg::edge_payloads_t const *edges = cfg->out_edge_payloads(node_from);
    assert(edges->size() == 1);
    cfg::edge const *e = edges->begin()->second;
    bool is_branch = false;
    bool is_direct;
    bool is_ret;
    edge_visitor ev;
    ev._([&](stmt_edge const *se) {
      statement_visitor v;
      v._([&](branch *i) {
        is_branch = true;
        is_ret = i->get_hint() == BRANCH_HINT_RET;
        rreil_evaluator rev;
        tie(is_direct, ignore) = rev.evaluate(i->get_target()->get_lin());
      });
      se->get_stmt()->accept(v);
    });
    e->accept(ev);
    assert(is_branch);
    if(is_ret)
      continue;

    if(!is_direct) {
      size_t machine_address = jd_manager.machine_address_of(node_from);
      set<size_t> _new;
      for(auto address : nt_it.second) {
        int_t ip = gdsl.get_ip();
        bool seekable = !gdsl.seek(address);
//        cout << hex << address << " " << dec << seekable << endl;
        assert(!gdsl.seek(ip));
        if(seekable) _new.insert(address);
      }

      auto current_it = address_targets.find(machine_address);
      if(current_it != address_targets.end()) {
        set<size_t> intersection;
        set_intersection(current_it->second.begin(), current_it->second.end(), _new.begin(), _new.end(),
          inserter(intersection, intersection.begin()));
        address_targets[machine_address] = intersection;

      } else
        address_targets[machine_address] = _new;

      if(address_targets[machine_address].size() == 0)
        cout << "No targets for jump at 0x" << hex << machine_address << dec << endl;
    }
  }
}

branch_statistics_data_t branch_statistics::get_stats() {
  size_t total = address_targets.size();
  size_t with_targets = 0;
  for(auto at_it : address_targets) {
    if(at_it.second.size() > 0) with_targets++;
  }
  return branch_statistics_data_t{total, with_targets};
}
