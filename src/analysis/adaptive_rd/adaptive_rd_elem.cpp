/*
 * lattice_elem.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/adaptive_rd/adaptive_rd_elem.h>
#include <cppgdsl/rreil/rreil.h>
#include <set>
#include <algorithm>
#include <memory>
#include <tuple>
#include <iostream>

using namespace analysis;
using namespace std;
using namespace gdsl::rreil;


adaptive_rd::adaptive_rd_elem *analysis::adaptive_rd::adaptive_rd_elem::lub(::analysis::lattice_elem *other,
    size_t current_node) {
  adaptive_rd_elem *other_casted = dynamic_cast<adaptive_rd_elem*>(other);

  elements_t explicit_undef;
  auto explicitify = [&](elements_t const &from, elements_t const &subst) {
    for(auto &mapping_from : from) {
      auto mapping_subst = subst.find(mapping_from.first);
      if(mapping_subst == subst.end()) explicit_undef[mapping_from.first] = 0;
    }
  };
  if(contains_undef)
    explicitify(other_casted->elements, this->elements);
  if(other_casted->contains_undef)
    explicitify(this->elements, other_casted->elements);

  elements_t lubbed = elements;

  auto explicit_lub = [&](elements_t const &from) {
    for(auto &mapping_other : from) {
      auto mapping_mine = lubbed.find(mapping_other.first);
      if(mapping_mine == lubbed.end()) lubbed[mapping_other.first] = mapping_other.second;
      else if(mapping_mine->second != mapping_other.second) lubbed[mapping_other.first] = current_node;
    }
  };
  explicit_lub(other_casted->elements);
  explicit_lub(explicit_undef);

  return new adaptive_rd_elem(contains_undef || other_casted->contains_undef, lubbed);
}

adaptive_rd::adaptive_rd_elem *analysis::adaptive_rd::adaptive_rd_elem::add(std::vector<singleton_t> elements) {
  elements_t added = this->elements;
  for(auto e : elements) {
    singleton_key_t k;
    singleton_value_t v;
    tie(k, v) = e;
    if(added.find(k) != added.end()) throw string("Element does already exist :/");
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

adaptive_rd::adaptive_rd_elem *analysis::adaptive_rd::adaptive_rd_elem::remove(
    std::function<bool(singleton_key_t, singleton_value_t)> pred) {
  elements_t removed;
  for(auto &e : this->elements)
    if(!pred(e.first, e.second))
      removed.insert(e);
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
  if(contains_undef) out << "+";
}
