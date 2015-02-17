/*
 * vs_top.h
 *
 *  Created on: Feb 12, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include "value_set.h"
#include "value_set_visitor.h"

namespace summy {

class vs_top: public value_set {
private:
  void put(std::ostream &out);
public:
  vs_top() {
  }

  bool smaller_equals(vs_finite const *vsf) const;
  bool smaller_equals(vs_open const *vsf) const;

  vs_shared_t narrow(vs_finite const *vsf) const;
  vs_shared_t narrow(vs_open const *vsf) const;

  vs_shared_t widen(vs_finite const *vsf) const;
  vs_shared_t widen(vs_open const *vsf) const;

  vs_shared_t join(vs_finite const *vsf) const;
  vs_shared_t join(vs_open const *vsf) const;

  void accept(value_set_visitor &v);
};

}
