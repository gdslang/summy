/*
 * dectran.h
 *
 *  Created on: Oct 30, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/cfg/cfg.h>
#include <summy/transformers/trivial_connector.h>
#include <cppgdsl/gdsl.h>
#include <experimental/optional>

#include <set>

class dectran {
private:
  cfg::cfg &cfg;
  bool blockwise_optimized;

protected:
  gdsl::gdsl &gdsl;

  cfg::translated_program_t decode_translate(bool decode_multiple);
  virtual size_t initial_cfg(
    cfg::cfg &cfg, bool decode_multiple, std::experimental::optional<std::string> name = std::experimental::nullopt);
public:
  dectran(cfg::cfg &cfg, gdsl::gdsl &g, bool blockwise_optimized);
};
