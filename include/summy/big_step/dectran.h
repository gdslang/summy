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
#include <optional>

#include <set>
#include <map>
#include <cppgdsl/gdsl.h>

typedef std::map<size_t, std::string> function_map_t;

struct dec_interval {
  int_t from;
  int_t to;

  dec_interval(int_t from, int_t to);
  bool operator <(dec_interval const& other) const;
//  bool operator <(int_t const& v) const;
};

class dectran {
private:
  cfg::cfg &cfg;
  bool blockwise_optimized;

  std::set<dec_interval> decoded_intervals;

  function_map_t fmap;

  size_t decode_iterations;
protected:
  bool speculative_decoding;
  gdsl::gdsl &gdsl;

  std::vector<std::tuple<uint64_t, gdsl::rreil::statements_t>> decode_translate(bool decode_multiple);
  virtual size_t initial_cfg(
    cfg::cfg &cfg, bool decode_multiple, std::optional<std::string> name = std::nullopt);
public:
  dectran(cfg::cfg &cfg, gdsl::gdsl &g, bool blockwise_optimized, bool speculative_decoding, function_map_t fmap);
  dectran(cfg::cfg &cfg, gdsl::gdsl &g, bool blockwise_optimized, bool speculative_decoding);
  virtual ~dectran() {
  }

  size_t get_decode_iterations() {
    return decode_iterations;
  }

  void print_decoding_holes();
  int_t bytes_decoded();
  int_t start_addresses_decoded();
};
