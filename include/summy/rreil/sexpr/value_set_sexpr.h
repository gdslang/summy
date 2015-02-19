/*
 * value_set.h
 *
 *  Created on: Feb 11, 2015
 *      Author: jucs
 */

#pragma once
#include <cppgdsl/rreil/sexpr/sexpr.h>
#include <summy/value_set/value_set.h>
#include <iostream>

namespace summy {
namespace rreil {

class value_set_sexpr: public gdsl::rreil::sexpr {
private:
  vs_shared_t inner;

  void put(std::ostream &out);
public:
  value_set_sexpr(vs_shared_t inner) :
      inner(inner) {
  }
  ~value_set_sexpr();

  const vs_shared_t &get_inner() const {
    return inner;
  }

  void accept(gdsl::rreil::sexpr_visitor &v);

};

}
}
