/*
 * fcollect_dectran.h
 *
 *  Created on: Dec 10, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/big_step/big_step.h>
#include <summy/cfg/cfg.h>
#include <summy/transformers/trivial_connector.h>
#include <cppgdsl/gdsl.h>
#include <experimental/optional>
#include <summy/big_step/dectran.h>

#include <set>

class fcollect_dectran : public dectran {
private:
  cfg::cfg cfg;

  size_t initial_cfg(
    cfg::cfg &cfg, bool decode_multiple, std::experimental::optional<std::string> name = std::experimental::nullopt);

public:
  fcollect_dectran(gdsl::gdsl &g, bool blockwise_optimized);

  cfg::cfg &get_cfg() {
    return cfg;
  }

  /*
   * Decode and translate first block
   */
  void transduce();
};
