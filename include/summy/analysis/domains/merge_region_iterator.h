/*
 * merge_region_iterator.h
 *
 *  Created on: Jun 3, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/memstate_util.h>
#include <iterator>
#include <optional>

namespace analysis {

struct field_desc_t {
  bool region_first;
  int64_t offset;
  field const f;
};

struct region_pair_desc_t {
  bool collision;

  std::optional<field_desc_t> field_first_region();
  std::optional<field_desc_t> field_second_region();

  field_desc_t ending_first;
  std::optional<field_desc_t> ending_last;
};

class merge_region_iterator;
bool operator==(const merge_region_iterator &a, const merge_region_iterator &b);
bool operator!=(const merge_region_iterator &a, const merge_region_iterator &b);

class merge_region_iterator : std::iterator<std::forward_iterator_tag, region_pair_desc_t> {
private:
  region_t::const_iterator r1_it;
  region_t::const_iterator r1_it_end;
  bool r1_collision;
  region_t::const_iterator r2_it;
  region_t::const_iterator r2_it_end;
  bool r2_collision;

public:
  merge_region_iterator(region_t::const_iterator r1_it, region_t::const_iterator r1_it_end,
    region_t::const_iterator r2_it, region_t::const_iterator r2_it_end);
  merge_region_iterator(region_t const &r1, region_t const &r2);
  static merge_region_iterator end(region_t const &r1, region_t const &r2);
  merge_region_iterator end();

  region_pair_desc_t operator*();
  merge_region_iterator &operator++();

  friend bool analysis::operator==(const merge_region_iterator &a, const merge_region_iterator &b);
  friend bool operator!=(const merge_region_iterator &a, const merge_region_iterator &b);
};
}
