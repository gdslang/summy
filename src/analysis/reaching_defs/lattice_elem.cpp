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
  definitions_t union_ids;
  set_union(ids.begin(), ids.end(), other_casted->ids.begin(), other_casted->ids.end(), inserter(union_ids, union_ids.begin()));
  return new lattice_elem(union_ids);
}

::analysis::reaching_defs::lattice_elem *analysis::reaching_defs::lattice_elem::add(definitions_t ids) {
//  set<id*> union_ids;
//  set_union(this->ids.begin(), this->ids.end(), ids.begin(), ids.end(), inserter(union_ids, union_ids.begin()));
//  return new lattice_elem(union_ids);
}

std::ostream &analysis::reaching_defs::operator <<(std::ostream &out, lattice_elem &_this) {
  out << "{";
  size_t i = 0;
  for(auto it = _this.ids.begin(); it != _this.ids.end(); it++, i++) {
    size_t node;
    id *_id;
    tie(node, _id) = *it;
    out << "(" << node << ", " << _id << ")" << (i < _this.ids.size() - 1 ?  ", " : "");
  }
  out << "}";
  return out;
}

#include <iostream>

bool analysis::reaching_defs::lattice_elem::operator >(::analysis::lattice_elem &other) {
  lattice_elem &other_casted = dynamic_cast<lattice_elem&>(other);
  return includes(ids.begin(), ids.end(), other_casted.ids.begin(), other_casted.ids.end());
}

bool analysis::reaching_defs::singleton_less::operator ()(singleton_t a, singleton_t b) {
  size_t a_node;
  id *a_id;
  tie(a_node, a_id) = a;
  size_t b_node;
  id *b_id;
  tie(b_node, b_id) = a;
  if(a_node < b_node)
    return true;
  else if(a_node > b_node)
    return false;
  return a_id->to_string().compare(b_id->to_string()) < 0;
}
