/*
 * big_step.h
 *
 *  Created on: Oct 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/observer.h>
#include <summy/cfg/cfg.h>
#include <vector>
#include <set>
#include <tuple>

class big_step: public cfg::observer {
protected:
  cfg::cfg &cfg;
public:
  big_step(cfg::cfg &cfg);
  virtual ~big_step() {
  }

  /*
   * Todo: Trennen, register() völlig optional machen
   */
  virtual void transduce_and_register() = 0;

  static cfg::updates_t combine_updates(const cfg::updates_t edges_ana, const cfg::edge_set_t edges_fn);
};
