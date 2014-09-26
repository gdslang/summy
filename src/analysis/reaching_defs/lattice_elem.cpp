/*
 * lattice_elem.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/reaching_defs/lattice_elem.h>
#include <set>
#include <algorithm>
#include <memory>
#include <tuple>

#include <cppgdsl/rreil/rreil.h>

using namespace analysis;
using namespace std;
using namespace gdsl::rreil;

analysis::reaching_defs::lattice_elem::~lattice_elem() {
}

::analysis::reaching_defs::lattice_elem *reaching_defs::lattice_elem::lub(::analysis::lattice_elem *other) {
  reaching_defs::lattice_elem *other_casted = dynamic_cast<reaching_defs::lattice_elem*>(other);
  definitions_t union_defs;
  set_union(defs.begin(), defs.end(), other_casted->defs.begin(), other_casted->defs.end(), inserter(union_defs, union_defs.begin()));
  return new lattice_elem(union_defs);
}

::analysis::reaching_defs::lattice_elem *analysis::reaching_defs::lattice_elem::add(definitions_t defs) {
  definitions_t union_defs;
  set_union(this->defs.begin(), this->defs.end(), defs.begin(), defs.end(), inserter(union_defs, union_defs.begin()));
  return new lattice_elem(union_defs);
}

std::ostream &analysis::reaching_defs::operator <<(std::ostream &out, lattice_elem &_this) {
  out << "{";
  size_t i = 0;
  for(auto it = _this.defs.begin(); it != _this.defs.end(); it++, i++) {
    size_t node;
    shared_ptr<id> _id;
    tie(node, _id) = *it;
    out << "(" << node << ", " << *_id << ")" << (i < _this.defs.size() - 1 ?  ", " : "");
  }
  out << "}";
  return out;
}

#include <iostream>

bool analysis::reaching_defs::lattice_elem::operator >(::analysis::lattice_elem &other) {
  lattice_elem &other_casted = dynamic_cast<lattice_elem&>(other);
  return !includes(other_casted.defs.begin(), other_casted.defs.end(), defs.begin(), defs.end());
}

bool analysis::reaching_defs::singleton_less::operator ()(singleton_t a, singleton_t b) {
  size_t a_node;
  shared_ptr<id> a_id;
  tie(a_node, a_id) = a;
  size_t b_node;
  shared_ptr<id> b_id;
  tie(b_node, b_id) = b;
  if(a_node < b_node)
    return true;
  else if(a_node > b_node)
    return false;
  return a_id->to_string().compare(b_id->to_string()) < 0;
}
