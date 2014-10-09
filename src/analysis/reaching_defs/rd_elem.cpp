/*
 * lattice_elem.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/reaching_defs/rd_elem.h>
#include <set>
#include <algorithm>
#include <memory>
#include <tuple>

#include <cppgdsl/rreil/rreil.h>

using namespace analysis;
using namespace std;
using namespace gdsl::rreil;

analysis::reaching_defs::rd_elem::~rd_elem() {
}

::analysis::reaching_defs::rd_elem *reaching_defs::rd_elem::lub(::analysis::lattice_elem *other) {
  reaching_defs::rd_elem *other_casted = dynamic_cast<reaching_defs::rd_elem*>(other);
  definitions_t union_defs;
  set_union(defs.begin(), defs.end(), other_casted->defs.begin(), other_casted->defs.end(),
      inserter(union_defs, union_defs.begin()));
  return new rd_elem(union_defs);
}

::analysis::reaching_defs::rd_elem *analysis::reaching_defs::rd_elem::add(definitions_t defs) {
  definitions_t union_defs;
  set_union(this->defs.begin(), this->defs.end(), defs.begin(), defs.end(), inserter(union_defs, union_defs.begin()));
  return new rd_elem(union_defs);
}

#include <iostream>

::analysis::reaching_defs::rd_elem *analysis::reaching_defs::rd_elem::remove(id_set_t subtrahend) {
//  cout << "{";
//  for(auto x : this->defs) {
//    size_t a_node;
//    shared_ptr<id> a_id;
//    tie(a_node, a_id) = x;
//    cout << "(" << a_node << ", " << a_id->to_string() << "), ";
//  }
//  cout << "} \\ {";
//  for(auto x : subtrahend)
//    cout << x->to_string() << ", ";
//  cout << "} = " << endl;

  definitions_t difference_defs;
  for(auto def : this->defs) {
    shared_ptr<id> a_id;
    tie(ignore, a_id) = def;
    if(subtrahend.find(a_id) == subtrahend.end())
      difference_defs.insert(def);
  }

//  cout << "{";
//  for(auto x : difference_defs) {
//    size_t a_node;
//    shared_ptr<id> a_id;
//    tie(a_node, a_id) = x;
//    cout << "(" << a_node << ", " << a_id->to_string() << "), ";
//  }
//  cout << "}" << endl;
//  cout << "+++++++++++++++++++++" << endl;

  return new rd_elem(difference_defs);
}

std::ostream &analysis::reaching_defs::operator <<(std::ostream &out, rd_elem &_this) {
  out << "{";
  size_t i = 0;
  for(auto it = _this.defs.begin(); it != _this.defs.end(); it++, i++) {
    size_t node;
    shared_ptr<id> _id;
    tie(node, _id) = *it;
    out << "(" << node << ", " << *_id << ")" << (i < _this.defs.size() - 1 ? ", " : "");
  }
  out << "}";
  return out;
}


bool analysis::reaching_defs::rd_elem::operator >(::analysis::lattice_elem &other) {
  rd_elem &other_casted = dynamic_cast<rd_elem&>(other);
  return !includes(other_casted.defs.begin(), other_casted.defs.end(), defs.begin(), defs.end());
}

bool analysis::reaching_defs::singleton_less::operator ()(singleton_t a, singleton_t b) {
  size_t a_node;
  shared_ptr<id> a_id;
  tie(a_node, a_id) = a;
  size_t b_node;
  shared_ptr<id> b_id;
  tie(b_node, b_id) = b;

  int id_cmp = a_id->to_string().compare(b_id->to_string());
  if(id_cmp < 0) return true;
  if(id_cmp > 0) return false;
  return a_node < b_node;
}

bool analysis::reaching_defs::id_less::operator ()(std::shared_ptr<gdsl::rreil::id> a,
    std::shared_ptr<gdsl::rreil::id> b) {
  return a->to_string().compare(b->to_string()) < 0;
}
