/*
 * bfs_order.cpp
 *
 *  Created on: Jan 23, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/bfs_order/bfs_order.h>
#include <summy/cfg/bfs_iterator.h>
#include <string>

using namespace cfg;
using namespace std;
using namespace analysis;

bfs_order::bfs_order(::cfg::cfg *cfg) {
  state = state_t(cfg->node_count());
  size_t counter = 0;
  for(bfs_iterator bi = cfg->begin(); bi != cfg->end(); ++bi)
    state[(*bi)->get_id()] = counter++;
}

void bfs_order::update(const std::vector<::cfg::update> &updates) {
  throw string("Not implemented");
}

bfs_order_result_t analysis::bfs_order::result() {
  return bfs_order_result_t(state);
}
