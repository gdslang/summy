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

analysis::lattice_elem *reaching_defs::lattice_elem::lub(analysis::lattice_elem *other) {
  reaching_defs::lattice_elem *other_casted = dynamic_cast<reaching_defs::lattice_elem*>(other);
  set<id*> union_ids;
  set_union(ids.begin(), ids.end(), other_casted->ids.begin(), other_casted->ids.end(), inserter(union_ids, union_ids.begin()));
  return new lattice_elem(union_ids);
}
