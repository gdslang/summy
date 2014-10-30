/*
 * dectran.h
 *
 *  Created on: Oct 30, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/big_step/big_step.h>
#include <summy/cfg/cfg.h>
#include <cppgdsl/gdsl.h>

/*
 * Todo: Instead of merging cfgs, adapt the transformers
 * to work on parts of the cfg (using an bfs_iterator
 * that only considers reachable nodes, ...). This allows
 * to simply use "add_program" in order to add newly translated
 * parts of the program.
 */

class dectran : public big_step {
private:
  cfg::cfg cfg;
  gdsl::gdsl &gdsl;

  cfg::translated_program_t decode_translate();
  void initial_cfg(cfg::cfg &cfg);
public:
  dectran(gdsl::gdsl &g);

  cfg::cfg &get_cfg() {
    return cfg;
  }

  /*
   * Decode and translate first block
   */
  void transduce_and_register();
  void notify(std::vector<cfg::update> const &updates);
};
