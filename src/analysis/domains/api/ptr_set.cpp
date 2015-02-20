/*
 * ptr_set.cpp
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/ptr_set.h>
#include <summy/value_set/vs_compare.h>

using namespace analysis;
using namespace summy;

bool anaylsis::api::ptr::operator <(struct ptr other) {
  if(id_less_no_version()(id, other.id))
    return true;
  return vs_total_less()(offset, other.offset);
}
