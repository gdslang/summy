/*
 * value_set.h
 *
 *  Created on: Feb 11, 2015
 *      Author: jucs
 */

#pragma once
#include <cppgdsl/rreil/sexpr/sexpr.h>
#include <cppgdsl/rreil/sexpr/sexpr_visitor.h>
#include <iostream>

namespace summy {

class value_set {
private:
  virtual void put(std::ostream &out) = 0;
public:
  virtual ~value_set() {
  }
  friend std::ostream &operator<< (std::ostream &out, value_set &_this);
  virtual void accept(gdsl::rreil::sexpr_visitor &v) = 0;
};

std::ostream &operator<<(std::ostream &out, value_set &_this);

}
