/*
 * big_step.cpp
 *
 *  Created on: Oct 24, 2014
 *      Author: Julian Kranz
 */
#include <summy/big_step/big_step.h>
#include <tuple>

using namespace std;
using namespace cfg;

big_step::big_step(class cfg &cfg) :
    cfg(cfg) {
}

updates_t big_step::combine_updates(const updates_t edges_ana, const edge_set_t edges_fn) {
  vector<update> result = edges_ana;
  for(auto &e_fn : edges_fn) {
    size_t from;
    size_t to;
    tie(from, to) = e_fn;
    result.push_back( { update_kind::UPDATE, from, to });
  }
  return result;
}

void big_step::register_() {
  cfg.register_observer(this);
}
