/*
 * analysis_dectran.h
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

class analysis_dectran : public dectran, public big_step {
private:
  trivial_connector tc;
  cfg::cfg cfg;

  std::set<size_t> unresolved;
  std::set<size_t> f_heads;

  size_t initial_cfg(
    cfg::cfg &cfg, bool decode_multiple, std::experimental::optional<std::string> name = std::experimental::nullopt);

public:
  analysis_dectran(gdsl::gdsl &g, bool blockwise_optimized);

  std::set<size_t> const &get_unresolved() {
    return unresolved;
  }

  void resolve(size_t resolved) {
    unresolved.erase(resolved);
  }

  cfg::cfg &get_cfg() {
    return cfg;
  }

  std::set<size_t> const& get_f_heads() {
    return f_heads;
  }

  /*
   * Decode and translate first block
   */
  void transduce(
    bool decode_multiple, std::experimental::optional<std::string> function_name = std::experimental::nullopt);
  void transduce_function(
    size_t address, std::experimental::optional<std::string> function_name = std::experimental::nullopt);
  void transduce() {
    transduce(false);
  }
  void notify(std::vector<cfg::update> const &updates);
};
