/*
 * lv_elem.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/liveness/lv_elem.h>
#include <algorithm>
#include <iostream>

using gdsl::rreil::id;

using namespace std;
using namespace analysis::liveness;

lv_elem *analysis::liveness::lv_elem::lub(::analysis::lattice_elem *other) {
  lv_elem *other_casted = dynamic_cast<lv_elem*>(other);

  /*
   * Todo: Use set_symmetric_difference / set_intersection (?)
   */

  elements_t result;
  for(auto &mapping : elements) {
    auto other_mapping = other_casted->elements.find(mapping.first);
    if(other_mapping == other_casted->elements.end()) result.insert(mapping);
    else result[mapping.first] = mapping.second | other_mapping->second;
  }
  for(auto &mapping_other : other_casted->elements) {
    auto mapping = other_casted->elements.find(mapping_other.first);
    if(mapping == other_casted->elements.end()) result.insert(mapping_other);
  }

  return new lv_elem(result);
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

bool analysis::liveness::lv_elem::contains(singleton_t s) {
  return eset.contains(s);
}

std::ostream &analysis::liveness::operator <<(std::ostream &out, lv_elem &_this) {
  out << "{";
  size_t i = 0;
  for(auto it = _this.eset.get_elements().begin(); it != _this.eset.get_elements().end(); it++, i++) {
    out << **it << (i < _this.eset.get_elements().size() - 1 ? ", " : "");
  }
  out << "}";
  return out;
}
