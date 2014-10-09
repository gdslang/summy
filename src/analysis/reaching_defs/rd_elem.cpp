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
  for(auto def : this->elements) {
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
  for(auto it = _this.elements.begin(); it != _this.elements.end(); it++, i++) {
    size_t node;
    shared_ptr<id> _id;
    tie(node, _id) = *it;
    out << "(" << node << ", " << *_id << ")" << (i < _this.elements.size() - 1 ? ", " : "");
  }
  out << "}";
  return out;
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
