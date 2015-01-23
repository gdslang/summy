/*
 * bfs_order.h
 *
 *  Created on: Jan 23, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/cfg.h>

#include <vector>
#include <memory>

typedef std::vector<size_t> state_t;

class bfs_order {
private:
  state_t state;

  cfg::cfg *cfg;
public:
  bfs_order(cfg::cfg *cfg);
  void update(std::vector<::cfg::update> const &updates);
};
