/*
 * lv_elem.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/liveness/lv_elem.h>
#include <algorithm>

using namespace std;
using namespace analysis::liveness;

analysis::liveness::lv_elem::~lv_elem() {
}

lv_elem *analysis::liveness::lv_elem::lub(::analysis::lattice_elem *other) {
  lv_elem *other_casted = dynamic_cast<lv_elem*>(other);
  living_t union_living;
  set_union(living.begin(), living.end(), other_casted->living.begin(), other_casted->living.end(),
      inserter(union_living, union_living.begin()));
  return new lv_elem(union_living);
}

lv_elem *analysis::liveness::lv_elem::add(living_t living) {
  living_t union_living;
  set_union(living.begin(), living.end(), living.begin(), living.end(),
      inserter(union_living, union_living.begin()));
  return new lv_elem(union_living);
}

lv_elem *analysis::liveness::lv_elem::remove(living_t living) {
  living_t difference_living;
  set_difference(living.begin(), living.end(), living.begin(), living.end(),
      inserter(difference_living, difference_living.begin()));
  return new lv_elem(difference_living);
}

bool analysis::liveness::lv_elem::operator >(::analysis::lattice_elem &other) {
  lv_elem &other_casted = dynamic_cast<lv_elem&>(other);
  return !includes(other_casted.living.begin(), other_casted.living.end(), living.begin(), living.end());
}
