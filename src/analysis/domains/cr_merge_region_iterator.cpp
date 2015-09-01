/*
 * cr_merge_region_iterator.cpp
 *
 *  Created on: Jul 9, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/cr_merge_region_iterator.h>
#include <summy/analysis/domains/merge_region_iterator.h>
#include <summy/rreil/id/numeric_id.h>
#include <tuple>

using namespace std;
using namespace std::experimental;
using namespace summy::rreil;
using namespace analysis;

bool analysis::operator==(const cr_merge_region_iterator &a, const cr_merge_region_iterator &b) {
  return a.mri == b.mri;
}

analysis::cr_merge_region_iterator::cr_merge_region_iterator(merge_region_iterator mri)
    : mri(mri), region(*(region_t *)NULL), fn_from(nullopt), fn_to(0) {}

bool analysis::operator!=(const cr_merge_region_iterator &a, const cr_merge_region_iterator &b) {
  return a.mri != b.mri;
}

void analysis::cr_merge_region_iterator::insert_f() {
  if(fn_from) {
    field f = field{(size_t)(fn_to - fn_from.value()), numeric_id::generate()};
    region.insert(make_pair(fn_from.value(), f));
    fn_from = nullopt;
  }
}

analysis::cr_merge_region_iterator::cr_merge_region_iterator(merge_region_iterator mri, region_t &region)
    : mri(mri), region(region), fn_from(nullopt), fn_to(0) {}

analysis::cr_merge_region_iterator::cr_merge_region_iterator(region_t::const_iterator r1_it,
  region_t::const_iterator r1_it_end, region_t::const_iterator r2_it, region_t::const_iterator r2_it_end,
  region_t &region)
    : cr_merge_region_iterator(merge_region_iterator(r1_it, r1_it_end, r2_it, r2_it_end), region) {}

analysis::cr_merge_region_iterator::cr_merge_region_iterator(const region_t &r1, const region_t &r2, region_t &region)
    : cr_merge_region_iterator(merge_region_iterator(r1, r2), region) {}


cr_merge_region_iterator analysis::cr_merge_region_iterator::end(const region_t &r1, const region_t &r2) {
  return cr_merge_region_iterator(merge_region_iterator::end(r1, r2));
}

region_pair_desc_t analysis::cr_merge_region_iterator::operator*() {
  return *mri;
}

cr_merge_region_iterator &analysis::cr_merge_region_iterator::operator++() {
  ++mri;

  if(mri != mri.end()) {
    region_pair_desc_t rpd = *mri;
    if(rpd.collision) {
      if(rpd.ending_last) {
        field_desc_t fd_ending_first = rpd.ending_first;
        field_desc_t fd_ending_last = rpd.ending_last.value();

        fn_to = fd_ending_last.offset + fd_ending_last.f.size;
        if(!fn_from) fn_from = fd_ending_first.offset;
      }
    } else
      insert_f();
  }

  if(mri == mri.end())
    insert_f();

  return *this;
}
