/*
 * set_elem.h
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/lattice_elem.h>
#include <algorithm>
#include <set>

namespace analysis {

template<typename SINGLETON_T, typename SINGLETON_LESS>
class set_elem {
public:
  typedef std::set<SINGLETON_T, SINGLETON_LESS> elements_t;
private:
  elements_t elements;
public:
  explicit set_elem(elements_t elements) :
      elements(elements) {
  }
  set_elem(set_elem const &e) {
    this->elements = e.elements;
  }
  virtual ~set_elem() {
  }

  elements_t &get_elements() {
    return elements;
  }

  virtual set_elem lub(set_elem &other) {
    elements_t union_elements;
    std::set_union(elements.begin(), elements.end(), other.elements.begin(), other.elements.end(),
        std::inserter(union_elements, union_elements.begin()), SINGLETON_LESS());
    return set_elem(union_elements);
  }
  virtual set_elem add(elements_t &elements) {
    elements_t union_elements;
    std::set_union(this->elements.begin(), this->elements.end(), elements.begin(), elements.end(),
        std::inserter(union_elements, union_elements.begin()), SINGLETON_LESS());
    return set_elem(union_elements);
  }
  virtual set_elem remove(elements_t &elements) {
    elements_t diff_elements;
    std::set_difference(this->elements.begin(), this->elements.end(), elements.begin(), elements.end(),
        std::inserter(diff_elements, diff_elements.begin()), SINGLETON_LESS());
    return set_elem(diff_elements);
  }

  virtual bool operator>=(set_elem &other) {
//    return !std::includes(other_casted.elements.begin(), other_casted.elements.end(), elements.begin(), elements.end(),
//        SINGLETON_LESS());
    return std::includes(elements.begin(), elements.end(), other.elements.begin(), other.elements.end(),
        SINGLETON_LESS());
  }

//  friend std::ostream &operator<< (std::ostream &out, lattice_elem &_this);
};

//std::ostream &operator<<(std::ostream &out, lattice_elem &_this);

}
