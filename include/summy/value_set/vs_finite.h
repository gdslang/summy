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
  vs_finite(int64_t value) : elements({value}) {
  }
  vs_finite(vs_finite const &other) :
      elements(other.elements) {
  }

  int64_t min() const;
  int64_t max() const;
  bool is_bottom() const;

  const elements_t &get_elements() const {
    return elements;
  }

  vs_shared_t narrow(vs_finite const *vsf) const;
  vs_shared_t narrow(vs_open const *vsf) const;

  vs_shared_t widen(vs_finite const *vsf) const;
  vs_shared_t widen(vs_open const *vsf) const;

  vs_shared_t add(vs_finite const *vs) const;
  vs_shared_t add(vs_open const *vs) const;

  vs_shared_t neg() const;

  vs_shared_t mul(vs_finite const *vs) const;
  vs_shared_t mul(vs_open const *vs) const;

  vs_shared_t div(vs_finite const *vs) const;
  vs_shared_t div(vs_open const *vs) const;
  vs_shared_t div(vs_top const *vs) const;

  bool smaller_equals(vs_finite const *vsf) const;
  bool smaller_equals(vs_open const *vsf) const;

  vs_shared_t join(vs_finite const *vsf) const;
  vs_shared_t join(vs_open const *vsf) const;

  vs_shared_t meet(vs_open const *vsf) const;

  void accept(value_set_visitor &v);

  static vs_shared_t single(int64_t value);

  static vs_shared_t const zero;
  static size_t const max_growth;
};

}
