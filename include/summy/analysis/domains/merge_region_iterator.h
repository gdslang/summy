/*
 * merge_region_iterator.h
 *
 *  Created on: Jun 3, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/memstate_util.h>
#include <iterator>
#include <experimental/optional>

namespace analysis {

struct field_off_t {
  int64_t offset;
  field const &f;
};

struct region_pair_desc_t {
  bool collision;
  field_off_t current;
  std::experimental::optional<field_off_t> overlap;
};

class merge_region_iterator;
bool operator==(const merge_region_iterator &a, const merge_region_iterator &b);
bool operator!=(const merge_region_iterator &a, const merge_region_iterator &b);

class merge_region_iterator: std::iterator<std::forward_iterator_tag, region_pair_desc_t> {
private:
  region_t::iterator r1_it;
  bool foo() { return true; }
  region_t::iterator r1_it_end;
  region_t::iterator r2_it;
  region_t::iterator r2_it_end;
public:
  merge_region_iterator(region_t::iterator r1_it, region_t::iterator r1_it_end, region_t::iterator r2_it,
      region_t::iterator r2_it_end);
  merge_region_iterator(region_t &r1, region_t &r2);
  static merge_region_iterator end(region_t &r1, region_t &r2);

  region_pair_desc_t operator*();
  merge_region_iterator &operator++();

  friend bool analysis::operator==(const merge_region_iterator &a, const merge_region_iterator &b);
  friend bool operator!=(const merge_region_iterator &a, const merge_region_iterator &b);
};


}
