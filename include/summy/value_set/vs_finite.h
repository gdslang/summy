/*
 * vs_finite.h
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include "value_set.h"
#include <set>

namespace summy {

class vs_finite: public value_set {
private:
  std::set<int64_t> const elements;
public:
  vs_finite(std::set<int64_t> const elements) :
      elements(elements) {
  }

  const std::set<int64_t> &get_elements() const {
    return elements;
  }
};

}
