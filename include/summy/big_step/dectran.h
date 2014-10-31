/*
 * dectran.h
 *
 *  Created on: Oct 30, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/big_step/big_step.h>
#include <summy/cfg/cfg.h>
#include <summy/transformers/trivial_connector.h>
#include <cppgdsl/gdsl.h>

class dectran : public big_step {
private:
  cfg::cfg cfg;
  gdsl::gdsl &gdsl;
  trivial_connector tc;
  bool blockwise_optimized;

  cfg::translated_program_t decode_translate();
  void initial_cfg(cfg::cfg &cfg);
public:
  dectran(gdsl::gdsl &g, bool blockwise_optimized);

  cfg::cfg &get_cfg() {
    return cfg;
  }

  /*
   * Decode and translate first block
   */
  void transduce_and_register();
  void notify(std::vector<cfg::update> const &updates);
};
