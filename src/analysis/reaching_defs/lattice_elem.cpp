/*
 * lattice_elem.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/reaching_defs/lattice_elem.h>
#include <set>
#include <algorithm>
#include <cppgdsl/rreil/rreil.h>

using namespace analysis;
using namespace std;
using namespace gdsl::rreil;

::analysis::reaching_defs::lattice_elem *reaching_defs::lattice_elem::lub(::analysis::lattice_elem *other) {
  reaching_defs::lattice_elem *other_casted = dynamic_cast<reaching_defs::lattice_elem*>(other);
  set<id*> union_ids;
  set_union(ids.begin(), ids.end(), other_casted->ids.begin(), other_casted->ids.end(), inserter(union_ids, union_ids.begin()));
  return new lattice_elem(union_ids);
}

::analysis::reaching_defs::lattice_elem *analysis::reaching_defs::lattice_elem::add(std::set<gdsl::rreil::id*> ids) {
  set<id*> union_ids;
  set_union(this->ids.begin(), this->ids.end(), ids.begin(), ids.end(), inserter(union_ids, union_ids.begin()));
  return new lattice_elem(union_ids);
}

std::ostream &analysis::reaching_defs::operator <<(std::ostream &out, lattice_elem &_this) {
  out << "{";
  size_t i = 0;
  for(auto it = _this.ids.begin(); it != _this.ids.end(); it++, i++)
    out << **it << (i < _this.ids.size() - 1 ?  ", " : "");
  out << "}";
  return out;
}

#include <iostream>

bool analysis::reaching_defs::lattice_elem::operator >(::analysis::lattice_elem &other) {
  lattice_elem &other_casted = dynamic_cast<lattice_elem&>(other);
  return includes(ids.begin(), ids.end(), other_casted.ids.begin(), other_casted.ids.end());
}
