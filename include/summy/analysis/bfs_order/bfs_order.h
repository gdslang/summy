/*
 * bfs_order.h
 *
 *  Created on: Jan 23, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/cfg.h>
#include <summy/analysis/analysis_result.h>

#include <vector>
#include <memory>

namespace analysis {

typedef std::vector<size_t> state_t;
typedef ::analysis::analysis_result<state_t> bfs_order_result_t;

class bfs_order {
private:
  state_t state;

//  cfg::cfg *cfg;
public:
  bfs_order(cfg::cfg *cfg);
  void update(std::vector<::cfg::update> const &updates);
  bfs_order_result_t result();
};

}
