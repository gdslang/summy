/*
 * statistics.h
 *
 *  Created on: May 20, 2016
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/jd_manager.h>
#include <map>
#include <set>

#include <cppgdsl/gdsl.h>

struct branch_statistics_data_t {
  size_t calls_total_indirect;
  size_t calls_with_targets;

  size_t jmps_total_indirect;
  size_t jmps_with_targets;

  branch_statistics_data_t()
      : calls_total_indirect(0), calls_with_targets(0), jmps_total_indirect(0), jmps_with_targets(0) {}
};

struct targets_t {
  bool is_call;
  std::set<size_t> targets;

  targets_t(bool is_call, std::set<size_t> targets) : is_call(is_call), targets(targets) {}

  targets_t() : is_call(false), targets(std::set<size_t>()) {}
};

class branch_statistics {
private:
  //  gdsl::gdsl &gdsl;
  //  analysis::summary_dstack &sd;
  //  cfg::jd_manager &jd_manager;

  std::map<size_t, targets_t> address_targets;

public:
  branch_statistics(gdsl::gdsl &gdsl, analysis::summary_dstack &sd, cfg::jd_manager &jd_manager);

  branch_statistics_data_t get_stats();
};

struct condition_statistics_data_t {
  size_t total_conditions;
  size_t cmp_conditions;
};

class condition_statistics {
private:
  cfg::cfg &cfg;

public:
  condition_statistics(cfg::cfg &cfg) : cfg(cfg) {}

  condition_statistics_data_t get_stats();
};

class loc_statistics {
private:
  cfg::cfg &cfg;

public:
  loc_statistics(cfg::cfg &cfg) : cfg(cfg) {}

  size_t get_loc();
};
