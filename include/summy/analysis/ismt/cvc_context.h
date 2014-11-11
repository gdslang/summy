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
  CVC4::ExprManager manager;
  CVC4::SmtEngine smtEngine;
  var_map_t var_map;
public:
  cvc_context();

  CVC4::ExprManager &get_manager() {
    return manager;
  }
  CVC4::SmtEngine &get_smtEngine() {
    return smtEngine;
  }
  var_map_t &get_var_map() {
    return var_map;
  }
};
}  // namespace analysis


