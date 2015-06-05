/*
 * merge_region_iterator.cpp
 *
 *  Created on: Jun 3, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/merge_region_iterator.h>
#include <assert.h>

using namespace std;
using namespace std::experimental;
using namespace analysis;

/*
 * region_pair_desc
 */

std::experimental::optional<field_desc_t> region_pair_desc_t::field_first_region() {
  if(ending_first.region_first)
    return ending_first;
  else
    return ending_last;
}

std::experimental::optional<field_desc_t> region_pair_desc_t::field_second_region() {
  if(ending_first.region_first)
    return ending_last;
  else
    return ending_first;
}

/*
 * merge_region_itearator
 */

merge_region_iterator::merge_region_iterator(region_t::const_iterator r1_it, region_t::const_iterator r1_it_end,
    region_t::const_iterator r2_it, region_t::const_iterator r2_it_end) :
    r1_it(r1_it), r1_it_end(r1_it_end), r2_it(r2_it), r2_it_end(r2_it_end) {
}

merge_region_iterator::merge_region_iterator(region_t const &r1, region_t const &r2) :
    r1_it(r1.begin()), r1_it_end(r1.end()), r2_it(r2.begin()), r2_it_end(r2.end()) {
}

merge_region_iterator merge_region_iterator::end(region_t const &r1, region_t const &r2) {
  return merge_region_iterator(r1.end(), r1.end(), r2.end(), r2.end());
}

region_pair_desc_t merge_region_iterator::operator *() {
  if(r1_it != r1_it_end && r2_it != r2_it_end) {
    int64_t offset_a = r1_it->first;
    field const &f_a = r1_it->second;
    field_desc_t f_a_off = field_desc_t { true, offset_a, f_a };
    int64_t end_a = offset_a + f_a.size;

    int64_t offset_b = r2_it->first;
    field const &f_b = r2_it->second;
    field_desc_t f_b_off = field_desc_t { false, offset_b, f_b };
    int64_t end_b = offset_b + f_b.size;

    bool collision = false;
    bool perfect_overlap = false;
    if(offset_a < offset_b) {
      if(end_a > offset_b)
        collision = true;
    } else if(offset_b > offset_a) {
      if(end_b > offset_a)
        collision = true;
    } else {
      if(end_a != end_b)
        collision = true;
      else
        perfect_overlap = true;
    }

    if(collision) {
      if(end_a < end_b)
        return region_pair_desc_t { true, f_a_off, f_b_off };
      else {
        assert(end_b > end_a);
        return region_pair_desc_t { true, f_b_off, f_a_off };
      }
    }
    else {
      if(perfect_overlap)
        return region_pair_desc_t { false, f_a_off, f_b_off };
      else /* no overlap */
        if(end_a < end_b)
          return region_pair_desc_t { false, f_a_off, nullopt };
        else
          return region_pair_desc_t { false, f_b_off, nullopt };
    }
  } else if(r1_it != r1_it_end && r2_it == r2_it_end) {
    int64_t offset_a = r1_it->first;
    field const &f_a = r1_it->second;
    field_desc_t f_a_off = field_desc_t { true, offset_a, f_a };

    return region_pair_desc_t { false, f_a_off, nullopt };
  } else if(r1_it == r1_it_end && r2_it != r2_it_end) {
    int64_t offset_b = r2_it->first;
    field const &f_b = r2_it->second;
    field_desc_t f_b_off = field_desc_t { false, offset_b, f_b };

    return region_pair_desc_t { false, f_b_off, nullopt };
  } else
    assert(false);
}

merge_region_iterator &merge_region_iterator::operator ++() {
  if(r1_it != r1_it_end && r2_it != r2_it_end) {
    int64_t offset_a = r1_it->first;
    field const &f_a = r1_it->second;
    int64_t end_a = offset_a + f_a.size;
    int64_t offset_b = r2_it->first;
    field const &f_b = r2_it->second;
    int64_t end_b = offset_b + f_b.size;

    if(offset_a == offset_b && end_a == end_b) {
      r1_it++;
      r2_it++;
    } else if(end_a < end_b) r1_it++;
    else r2_it++;
  } else if(r1_it == r1_it_end && r2_it != r2_it_end) r2_it++;
  else if(r1_it != r1_it_end && r2_it == r2_it_end) r1_it++;
  else
  assert(false);
  return *this;
}

bool analysis::operator ==(const merge_region_iterator &a, const merge_region_iterator &b) {
  return (a.r1_it == b.r1_it) && (a.r2_it == b.r2_it);
}

bool analysis::operator !=(const merge_region_iterator &a, const merge_region_iterator &b) {
  return !(a == b);
}
