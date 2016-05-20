/*
 * statistics.h
 *
 *  Created on: May 20, 2016
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/cfg/jd_manager.h>
#include <map>
#include <set>

#include <cppgdsl/gdsl.h>

struct branch_statistics_data_t {
  size_t total_indirect;
  size_t with_targets;
};

class branch_statistics {
private:
  gdsl::gdsl &gdsl;
  analysis::summary_dstack &sd;
  cfg::jd_manager &jd_manager;

  std::map<size_t, std::set<size_t>> address_targets;

public:
  branch_statistics(gdsl::gdsl &gdsl, analysis::summary_dstack &sd, cfg::jd_manager &jd_manager);

  branch_statistics_data_t get_stats();
};
