/*
 * Copyright 2014-2016 Julian Kranz, Technical University of Munich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * lattice_elem.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <algorithm>
#include <cppgdsl/rreil/rreil.h>
#include <iosfwd>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <summy/analysis/adaptive_rd/adaptive_rd_state.h>
#include <summy/analysis/util.h>
#include <tuple>

using namespace analysis;
using namespace std;
using namespace gdsl::rreil;

bool analysis::adaptive_rd::singleton_equals(const singleton_t &a, const singleton_t &b) {
  singleton_key_t a_k;
  singleton_value_t a_v;
  tie(a_k, a_v) = a;
  singleton_key_t b_k;
  singleton_value_t b_v;
  tie(b_k, b_v) = b;
  return print_id_no_version(*a_k) == print_id_no_version(*b_k) && a_v == b_v;
}

adaptive_rd::adaptive_rd_state *analysis::adaptive_rd::adaptive_rd_state::join(
  ::analysis::domain_state *other, size_t current_node) {
  adaptive_rd_state *other_casted = dynamic_cast<adaptive_rd_state *>(other);

  elements_t explicit_undef;
  auto explicitify = [&](elements_t const &from, elements_t const &subst) {
    for(auto &mapping_from : from) {
      auto mapping_subst = subst.find(mapping_from.first);
      if(mapping_subst == subst.end()) explicit_undef[mapping_from.first] = 0;
    }
  };
  if(contains_undef) explicitify(other_casted->elements, this->elements);
  if(other_casted->contains_undef) explicitify(this->elements, other_casted->elements);

  elements_t lubbed = elements;

  auto explicit_lub = [&](elements_t const &from) {
    for(auto &mapping_other : from) {
      auto mapping_mine = lubbed.find(mapping_other.first);
      if(mapping_mine == lubbed.end())
        lubbed[mapping_other.first] = mapping_other.second;
      else if(mapping_mine->second != mapping_other.second)
        lubbed[mapping_other.first] = current_node;
    }
  };
  explicit_lub(other_casted->elements);
  explicit_lub(explicit_undef);

  size_t memory_rev = this->memory_rev;
  if(memory_rev != other_casted->memory_rev) memory_rev = current_node;

  return new adaptive_rd_state(contains_undef || other_casted->contains_undef, lubbed, memory_rev);
}

adaptive_rd::adaptive_rd_state *analysis::adaptive_rd::adaptive_rd_state::narrow(
  ::analysis::domain_state *other, size_t) {
  return new adaptive_rd::adaptive_rd_state(*dynamic_cast<adaptive_rd::adaptive_rd_state *>(other));
}

adaptive_rd::adaptive_rd_state *analysis::adaptive_rd::adaptive_rd_state::widen(
  ::analysis::domain_state *other, size_t) {
  return new adaptive_rd::adaptive_rd_state(*dynamic_cast<adaptive_rd::adaptive_rd_state *>(other));
}

adaptive_rd::adaptive_rd_state *analysis::adaptive_rd::adaptive_rd_state::add(
  std::vector<singleton_t> elements) {
  elements_t added = this->elements;
  for(auto e : elements) {
    singleton_key_t k;
    singleton_value_t v;
    tie(k, v) = e;
    if(added.find(k) != added.end()) throw string("Element does already exist :/");
    added[k] = v;
  }
  return new adaptive_rd_state(contains_undef, added, memory_rev);
}
adaptive_rd::adaptive_rd_state *analysis::adaptive_rd::adaptive_rd_state::remove(
  id_set_t elements) {
  elements_t removed = this->elements;
  for(auto id : elements)
    removed.erase(id);
  return new adaptive_rd_state(contains_undef, removed, memory_rev);
}

adaptive_rd::adaptive_rd_state *analysis::adaptive_rd::adaptive_rd_state::remove(
  std::function<bool(singleton_key_t, singleton_value_t)> pred) {
  elements_t removed;
  for(auto &e : this->elements)
    if(!pred(e.first, e.second)) removed.insert(e);
  return new adaptive_rd_state(contains_undef, removed, memory_rev);
}

adaptive_rd::adaptive_rd_state *analysis::adaptive_rd::adaptive_rd_state::set_memory_rev(
  size_t memory_rev) {
  return new adaptive_rd_state(contains_undef, elements, memory_rev);
}

bool analysis::adaptive_rd::adaptive_rd_state::operator>=(
  ::analysis::domain_state const &other) const {
  adaptive_rd_state const &other_casted = dynamic_cast<adaptive_rd_state const &>(other);
  if(contains_undef && !other_casted.contains_undef) return true;
  if(!contains_undef && other_casted.contains_undef) return false;
  if(memory_rev != other_casted.memory_rev) return false;
  return equal(this->elements.begin(), this->elements.end(), other_casted.elements.begin(),
    other_casted.elements.end(), singleton_equals);
}

void analysis::adaptive_rd::adaptive_rd_state::put(std::ostream &out) const {
  out << "{";
  size_t i = 0;
  for(auto it = elements.begin(); it != elements.end(); it++, i++) {
    out << *it->first << " -> " << it->second << (i < elements.size() - 1 ? ", " : "");
  }
  out << " | memory -> " << memory_rev << "}";
  if(contains_undef) out << "+";
}
