/*
 * cr_merge_region_iterator.h
 *
 * A special version of the merge_region_iterator that replaces fields
 * when a conflict is encountered
 *
 *  Created on: Jul 9, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/memstate_util.h>
#include <iterator>
#include <experimental/optional>
#include "merge_region_iterator.h"
#include <experimental/optional>

namespace analysis {

class cr_merge_region_iterator;
bool operator==(const cr_merge_region_iterator &a, const cr_merge_region_iterator &b);
bool operator!=(const cr_merge_region_iterator &a, const cr_merge_region_iterator &b);

class cr_merge_region_iterator : std::iterator<std::forward_iterator_tag, region_pair_desc_t> {
private:
  merge_region_iterator mri;

  region_t &region;
  std::experimental::optional<int64_t> fn_from;
  int64_t fn_to;

  void insert_f();

  cr_merge_region_iterator(merge_region_iterator mri);

public:
  cr_merge_region_iterator(merge_region_iterator mri, region_t &region);
  cr_merge_region_iterator(region_t::const_iterator r1_it, region_t::const_iterator r1_it_end,
    region_t::const_iterator r2_it, region_t::const_iterator r2_it_end, region_t &region);
  cr_merge_region_iterator(region_t const &r1, region_t const &r2, region_t &region);
  static cr_merge_region_iterator end(region_t const &r1, region_t const &r2);

  region_pair_desc_t operator*();
  cr_merge_region_iterator &operator++();

  friend bool analysis::operator==(const cr_merge_region_iterator &a, const cr_merge_region_iterator &b);
  friend bool operator!=(const cr_merge_region_iterator &a, const cr_merge_region_iterator &b);
};
}
