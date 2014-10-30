/*
 * dectran.h
 *
 *  Created on: Oct 30, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/big_step/big_step.h>
#include <cppgdsl/gdsl.h>

class dectran : public big_step {
private:
public:
  dectran(cfg::cfg &cfg, gdsl::gdsl &g);

  /*
   * Decode and translate first block
   */
  void transduce_and_register();
  void notify(std::vector<cfg::update> const &updates);
};
