/*
 * lv_elem.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/liveness/lv_state.h>

#include <cppgdsl/rreil/id/id.h>
#include <algorithm>
#include <tuple>
#include <iostream>
#include <vector>

using gdsl::rreil::id;

using namespace std;
using namespace analysis::liveness;

lv_state *analysis::liveness::lv_state::join(::analysis::domain_state *other, size_t current_node) {
  lv_state *other_casted = dynamic_cast<lv_state*>(other);

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
    auto mapping = elements.find(mapping_other.first);
    if(mapping == elements.end()) result.insert(mapping_other);
  }

  return new lv_state(result);
}

lv_state *analysis::liveness::lv_state::box(::analysis::domain_state *other, size_t current_node) {
  return new lv_state(*dynamic_cast<lv_state*>(other));
}

lv_state *analysis::liveness::lv_state::add(std::vector<singleton_t> elements) {
  elements_t current = this->elements;
  for(auto &mapping : elements) {
    singleton_key_t key;
    singleton_value_t value;
    tie(key, value) = mapping;

    auto current_mapping = current.find(key);
    if(current_mapping == current.end()) current[key] = value;
    else current[key] = current_mapping->second | value;
  }

  return new lv_state(current);
}

lv_state *analysis::liveness::lv_state::remove(std::vector<singleton_t> elements) {
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
  return new lv_state(elements_removed);
}

bool analysis::liveness::lv_state::operator >=(::analysis::domain_state const &other) const {
  lv_state const &other_casted = dynamic_cast<lv_state const&>(other);
  for(auto &mapping_other : other_casted.elements) {
    auto mapping = elements.find(mapping_other.first);
    if(mapping == elements.end()) return false;
    else if(mapping_other.second & mapping->first != mapping->first) return false;
  }
  return true;
}

bool analysis::liveness::lv_state::contains_bit(singleton_t s) {
  singleton_key_t id;
  singleton_value_t bits;
  tie(id, bits) = s;
  auto mapping = elements.find(id);
  if(mapping == elements.end())
    return false;
  return mapping->second & bits;
}

void analysis::liveness::lv_state::put(std::ostream &out) const {
  out << "{";
  size_t i = 0;
  for(auto it = elements.begin(); it != elements.end(); it++, i++) {
    out << "(" << *it->first << ", " << hex << it->second << dec << ")" << (i < elements.size() - 1 ? ", " : "");
  }
  out << "}";
}
