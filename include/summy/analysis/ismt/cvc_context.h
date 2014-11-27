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
typedef std::map<size_t, CVC4::Expr> memory_map_t;

class cvc_context {
private:
  CVC4::ExprManager manager;
  CVC4::SmtEngine smtEngine;
  var_map_t var_map;

  CVC4::DatatypeType value_type;
  CVC4::Expr val_ctor;
  CVC4::Expr val_sel;

  CVC4::ArrayType mem_type;
  memory_map_t mem_map;
public:
  cvc_context(bool unsat_cores);

  CVC4::ExprManager &get_manager() {
    return manager;
  }
  CVC4::SmtEngine &get_smtEngine() {
    return smtEngine;
  }
  CVC4::Expr var(std::string name);
  CVC4::Expr memory(size_t rev);

  CVC4::Expr add(CVC4::Expr a, CVC4::Expr b);

  CVC4::Expr pack(CVC4::Expr expr);
  CVC4::Expr unpack(CVC4::Expr expr);
};
}  // namespace analysis


