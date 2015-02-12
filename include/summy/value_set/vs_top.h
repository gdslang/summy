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

  vs_shared_t join(vs_finite const *vsf);
  vs_shared_t join(vs_open const *vsf);

  void accept(value_set_visitor &v);
};

}
