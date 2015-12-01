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
#include <experimental/optional>

#include <set>

class dectran : public big_step {
private:
  cfg::cfg cfg;
  gdsl::gdsl &gdsl;
  trivial_connector tc;
  bool blockwise_optimized;

  std::set<size_t> unresolved;

  cfg::translated_program_t decode_translate(bool decode_multiple);
  size_t initial_cfg(
    cfg::cfg &cfg, bool decode_multiple, std::experimental::optional<std::string> name = std::experimental::nullopt);

public:
  dectran(gdsl::gdsl &g, bool blockwise_optimized);

  cfg::cfg &get_cfg() {
    return cfg;
  }

  std::set<size_t> const &get_unresolved() {
    return unresolved;
  }

  void resolve(size_t resolved) {
    unresolved.erase(resolved);
  }

  /*
   * Decode and translate first block
   */
  void transduce(bool decode_multiple, std::experimental::optional<std::string> function_name = std::experimental::nullopt);
  void transduce_function(size_t address, std::string function_name);
  void transduce() {
    transduce(false);
  }
  void notify(std::vector<cfg::update> const &updates);
};
