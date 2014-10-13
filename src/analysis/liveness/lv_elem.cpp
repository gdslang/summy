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

lv_elem *analysis::liveness::lv_elem::lub(::analysis::lattice_elem *other) {
  lv_elem *other_casted = dynamic_cast<lv_elem*>(other);
  auto lubbed = eset.lub(other_casted->eset);
  return new lv_elem(lubbed);
}

lv_elem *analysis::liveness::lv_elem::add(elements_t elements) {
  auto added = eset.add(elements);
  return new lv_elem(added);
}

lv_elem *analysis::liveness::lv_elem::remove(elements_t elements) {
  auto removed = eset.remove(elements);
  return new lv_elem(removed);
}

bool analysis::liveness::lv_elem::operator >=(::analysis::lattice_elem &other) {
  lv_elem &other_casted = dynamic_cast<lv_elem&>(other);
  return eset >= other_casted.eset;
}
