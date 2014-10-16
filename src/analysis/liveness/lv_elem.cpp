/*
 * lv_elem.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/liveness/lv_elem.h>
#include <algorithm>
#include <tuple>
#include <iostream>
#include <vector>

using gdsl::rreil::id;

using namespace std;
using namespace analysis::liveness;

lv_elem *analysis::liveness::lv_elem::lub(::analysis::lattice_elem *other, size_t current_node) {
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

//  cout << "lubbed" << *(new lv_elem(result)) << endl;

  return new lv_elem(result);
}

lv_elem *analysis::liveness::lv_elem::add(std::vector<singleton_t> elements) {
  elements_t current = this->elements;
  for(auto &mapping : elements) {
    singleton_key_t key;
    singleton_value_t value;
    tie(key, value) = mapping;

//    cout << "kv/add " << *key << ", " << value << endl;

    auto current_mapping = current.find(key);
    if(current_mapping == current.end()) current[key] = value;
    else current[key] = current_mapping->second | value;
  }

//  cout << *(new lv_elem(current)) << endl;

  return new lv_elem(current);
}

lv_elem *analysis::liveness::lv_elem::remove(std::vector<singleton_t> elements) {
  elements_t elements_removed = this->elements;
  for(auto &mapping : elements) {
    singleton_key_t key;
    singleton_value_t value;
    tie(key, value) = mapping;
    auto current_mapping = elements_removed.find(key);
    if(current_mapping != elements_removed.end()) {
      singleton_value_t value_new = current_mapping->second & ~value;
      if(value_new)
        elements_removed[key] = value_new;
      else
        elements_removed.erase(key);
    }
  }
  return new lv_elem(elements_removed);
}

bool analysis::liveness::lv_elem::operator >=(::analysis::lattice_elem &other) {
  lv_elem &other_casted = dynamic_cast<lv_elem&>(other);
  for(auto &mapping_other : other_casted.elements) {
    auto mapping = elements.find(mapping_other.first);
    if(mapping == elements.end()) return false;
    else if(mapping_other.second & mapping->first != mapping->first) return false;
  }
  return true;
}

bool analysis::liveness::lv_elem::contains_bit(singleton_t s) {
  singleton_key_t id;
  singleton_value_t bits;
  tie(id, bits) = s;
  auto mapping = elements.find(id);
  if(mapping == elements.end())
    return false;
  return mapping->second & bits;
}

void analysis::liveness::lv_elem::put(std::ostream &out) {
  out << "{";
  size_t i = 0;
  for(auto it = elements.begin(); it != elements.end(); it++, i++) {
    out << "(" << *it->first << ", " << it->second << ")" << (i < elements.size() - 1 ? ", " : "");
  }
  out << "}";
}
