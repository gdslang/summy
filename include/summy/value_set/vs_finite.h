/*
 * vs_finite.h
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include "value_set.h"
#include <summy/value_set/value_set_visitor.h>
#include <set>

namespace summy {

class vs_finite: public value_set {
public:
  typedef std::set<int64_t> elements_t;
private:
  elements_t const elements;

  void put(std::ostream &out);
public:
  vs_finite() {
  }
  vs_finite(elements_t const elements) :
      elements(elements) {
  }

  const elements_t &get_elements() const {
    return elements;
  }

  vs_shared_t join(vs_finite const *vsf);
  vs_shared_t join(vs_open const *vsf);

  void accept(value_set_visitor &v);
};

}
