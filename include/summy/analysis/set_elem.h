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

template<typename SINGLETON_T, typename SINGLETON_LESS, typename SPEC_ELEM>
class set_elem: public lattice_elem {
public:
  typedef std::set<SINGLETON_T, SINGLETON_LESS> elements_t;
protected:
  elements_t elements;
public:
  set_elem(elements_t elements) :
      elements(elements) {
  }
  set_elem(set_elem &e) : lattice_elem(e) {
    this->elements = e.elements;
  }
  virtual ~set_elem() {
  }
  virtual set_elem *lub(::analysis::lattice_elem *other) {
    set_elem *other_casted = dynamic_cast<set_elem*>(other);
    elements_t union_elements;
    std::set_union(elements.begin(), elements.end(), other_casted->elements.begin(), other_casted->elements.end(),
        std::inserter(union_elements, union_elements.begin()), SINGLETON_LESS());
    return new SPEC_ELEM(union_elements);
  }
  virtual set_elem *add(elements_t elements) {
    elements_t union_elements;
    std::set_union(this->elements.begin(), this->elements.end(), elements.begin(), elements.end(),
        std::inserter(union_elements, union_elements.begin()), SINGLETON_LESS());
    return new SPEC_ELEM(union_elements);
  }
  virtual set_elem *remove(elements_t elements) {
    elements_t diff_elements;
    std::set_difference(this->elements.begin(), this->elements.end(), elements.begin(), elements.end(),
        std::inserter(diff_elements, diff_elements.begin()), SINGLETON_LESS());
    return new SPEC_ELEM(diff_elements);
  }

  virtual bool operator>=(::analysis::lattice_elem &other) {
    set_elem &other_casted = dynamic_cast<set_elem&>(other);
//    return !std::includes(other_casted.elements.begin(), other_casted.elements.end(), elements.begin(), elements.end(),
//        SINGLETON_LESS());
    return std::includes(elements.begin(), elements.end(), other_casted.elements.begin(), other_casted.elements.end(),
        SINGLETON_LESS());
  }

//  friend std::ostream &operator<< (std::ostream &out, lattice_elem &_this);
};

//std::ostream &operator<<(std::ostream &out, lattice_elem &_this);

}
