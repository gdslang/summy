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

  elements_t difference_defs;
  for(auto def : this->eset.get_elements()) {
    shared_ptr<id> a_id;
    tie(ignore, a_id) = def;
    if(subtrahend.find(a_id) == subtrahend.end()) difference_defs.insert(def);
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

  return new rd_elem(contains_undef, difference_defs);
}

reaching_defs::rd_elem *analysis::reaching_defs::rd_elem::lub(::analysis::lattice_elem *other, size_t current_node) {
  rd_elem *other_casted = dynamic_cast<rd_elem*>(other);

  id_set_t ids_mine;
  id_set_t ids_other;
  auto extract_ids = [&](id_set_t &dest, rd_elem::elements_t &elements) {
    for(auto def : elements) {
      shared_ptr<id> id;
      tie(ignore, id) = def;
      dest.insert(id);
    }
  };
  extract_ids(ids_mine, this->eset.get_elements());
  extract_ids(ids_other, other_casted->eset.get_elements());

//  cout << "Mine: ";
//  for(auto id : ids_mine)
//    cout << id->to_string() << ", ";
//  cout << endl;
//
//  cout << "Others: ";
//  for(auto id : ids_other)
//    cout << id->to_string() << ", ";
//  cout << endl;

  id_set_t ids_sym_diff;
//  set_symmetric_difference(ids_mine.begin(), ids_mine.end(), ids_other.begin(), ids_other.end(),
//      inserter(ids_sym_diff, ids_sym_diff.begin()), id_less());

  if(contains_undef) set_difference(ids_other.begin(), ids_other.end(), ids_mine.begin(), ids_mine.end(),
      inserter(ids_sym_diff, ids_sym_diff.begin()), id_less());
  if(other_casted->contains_undef) set_difference(ids_mine.begin(), ids_mine.end(), ids_other.begin(), ids_other.end(),
      inserter(ids_sym_diff, ids_sym_diff.begin()), id_less());

//  cout << "Sym Diff: ";
//  for(auto id : ids_sym_diff)
//    cout << id->to_string() << ", ";
//  cout << endl << endl;

  rd_elem::elements_t explicit_undef;
  for(auto id : ids_sym_diff)
    explicit_undef.insert(singleton_t(0, id));

  auto lubbed = eset.lub(other_casted->eset);
  auto result_set = lubbed.add(explicit_undef);
  return new rd_elem(contains_undef || other_casted->contains_undef, result_set);
}

reaching_defs::rd_elem *analysis::reaching_defs::rd_elem::add(elements_t elements) {
  auto added = eset.add(elements);
  return new rd_elem(contains_undef, added);
}

reaching_defs::rd_elem *analysis::reaching_defs::rd_elem::remove(elements_t elements) {
  auto removed = eset.remove(elements);
  return new rd_elem(contains_undef, removed);
}

reaching_defs::rd_elem *analysis::reaching_defs::rd_elem::remove(
    std::function<bool(size_t, std::shared_ptr<gdsl::rreil::id>)> pred) {
  elements_t removed;
  for(auto e : eset.get_elements()) {
    size_t def;
    shared_ptr<id> id;
    tie(def, id) = e;
    if(!pred(def, id))
      removed.insert(e);
  }
  return new rd_elem(contains_undef, removed);
}

bool analysis::reaching_defs::rd_elem::operator >=(::analysis::lattice_elem &other) {
  rd_elem &other_casted = dynamic_cast<rd_elem&>(other);
  if(contains_undef && !other_casted.contains_undef) return true;
  if(!contains_undef && other_casted.contains_undef) return false;
  return eset >= other_casted.eset;
}

void analysis::reaching_defs::rd_elem::put(std::ostream& out) {
  out << "{";
  size_t i = 0;
  for(auto it = eset.get_elements().begin(); it != eset.get_elements().end(); it++, i++) {
    size_t node;
    shared_ptr<id> _id;
    tie(node, _id) = *it;
    out << "(" << node << ", " << *_id << ")" << (i < eset.get_elements().size() - 1 ? ", " : "");
  }
  out << "}";
  if(contains_undef) out << "+";
}
