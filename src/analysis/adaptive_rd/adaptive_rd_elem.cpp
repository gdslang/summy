/*
 * lattice_elem.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/adaptive_rd/adaptive_rd_elem.h>
#include <set>
#include <algorithm>
#include <memory>
#include <tuple>

#include <cppgdsl/rreil/rreil.h>

using namespace analysis;
using namespace std;
using namespace gdsl::rreil;

#include <iostream>

//bool analysis::adaptive_rd::singleton_less::operator ()(singleton_t a, singleton_t b) {
//  size_t a_node;
//  shared_ptr<id> a_id;
//  tie(a_id, a_node) = a;
//  size_t b_node;
//  shared_ptr<id> b_id;
//  tie(b_id, b_node) = b;
//
//  int id_cmp = a_id->to_string().compare(b_id->to_string());
//  if(id_cmp < 0) return true;
//  if(id_cmp > 0) return false;
//  return a_node < b_node;
//}

adaptive_rd::adaptive_rd_elem *analysis::adaptive_rd::adaptive_rd_elem::lub(::analysis::lattice_elem *other, size_t current_node) {
  adaptive_rd_elem *other_casted = dynamic_cast<adaptive_rd_elem*>(other);

  elements_t lubbed = elements;
  for(auto &mapping_other : other_casted->elements) {
    auto mapping_mine = lubbed.find(mapping_other.first);
    if(mapping_mine == lubbed.end())
      lubbed[mapping_other.first] = mapping_other.second;
    else if(mapping_mine->second != mapping_other.second)
      lubbed[mapping_other.first] = current_node;
  }

  return new adaptive_rd_elem(contains_undef || other_casted->contains_undef, lubbed);
}

adaptive_rd::adaptive_rd_elem *analysis::adaptive_rd::adaptive_rd_elem::add(std::vector<singleton_t> elements) {
  elements_t added = this->elements;
  for(auto e : elements) {
    singleton_key_t k;
    singleton_value_t v;
    tie(k, v) = e;
    if(added.find(k) != added.end())
      throw string("Element does already exist :/");
    added[k] = v;
  }
  return new adaptive_rd_elem(contains_undef, added);
}
adaptive_rd::adaptive_rd_elem *analysis::adaptive_rd::adaptive_rd_elem::remove(id_set_t elements) {
  elements_t removed = this->elements;
  for(auto id : elements)
    removed.erase(id);
  return new adaptive_rd_elem(contains_undef, removed);
}

bool analysis::adaptive_rd::adaptive_rd_elem::operator >=(::analysis::lattice_elem &other) {
  adaptive_rd_elem &other_casted = dynamic_cast<adaptive_rd_elem&>(other);
  if(contains_undef && !other_casted.contains_undef) return true;
  if(!contains_undef && other_casted.contains_undef) return false;
  return this->elements == other_casted.elements;
}

void analysis::adaptive_rd::adaptive_rd_elem::put(std::ostream &out) {
  out << "{";
  size_t i = 0;
  for(auto it = elements.begin(); it != elements.end(); it++, i++) {
    out << "(" << *it->first << ", " << it->second << ")" << (i < elements.size() - 1 ? ", " : "");
  }
  out << "}";
  if(contains_undef)
    out << "+";
}
