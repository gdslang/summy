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
namespace rreil {


class value_set : public gdsl::rreil::sexpr {
private:
  void put(std::ostream &out);
public:
  void accept(gdsl::rreil::sexpr_visitor &v);
};

}
}
