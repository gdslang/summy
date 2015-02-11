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
private:
  std::set<int64_t> const elements;

  void put(std::ostream &out);
public:
  vs_finite(std::set<int64_t> const elements) :
      elements(elements) {
  }

  const std::set<int64_t> &get_elements() const {
    return elements;
  }

  void accept(value_set_visitor &v);
};

}
