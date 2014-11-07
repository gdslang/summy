/*
 * cvc_context.h
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <map>
#include <cvc4/cvc4.h>
#include <string>

namespace analysis {

typedef std::map<std::string, CVC4::Expr> var_map_t;

class cvc_context {
private:
  var_map_t var_map;
  CVC4::ExprManager manager;
public:
  var_map_t &get_var_map() {
    return var_map;
  }
  CVC4::ExprManager &get_manager() {
    return manager;
  }
};
}  // namespace analysis


