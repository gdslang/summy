/*
 * cvc_context.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/ismt/cvc_context.h>
#include <cvc4/cvc4.h>
#include <string>
#include <sstream>

using namespace std;
using namespace CVC4;

analysis::cvc_context::cvc_context(bool unsat_cores) :
    smtEngine(&manager) {
  smtEngine.setOption("produce-models", SExpr("true"));
  smtEngine.setOption("incremental", SExpr("true"));
  if(unsat_cores) {
    smtEngine.setOption("produce-unsat-cores", SExpr("true"));
//    smtEngine.setOption("tear-down-incremental", SExpr("true"));
  }

  CVC4::Datatype value("value");

  DatatypeConstructor val("VAL");
  val.addArg("value", manager.mkBitVectorType(64));
  value.addConstructor(val);

  this->value_type = manager.mkDatatypeType(value);
  this->val_ctor = value_type.getDatatype()[0].getConstructor();
  this->val_sel = value_type.getDatatype()[0].getSelector("value");

  mem_type = manager.mkArrayType(manager.mkBitVectorType(61), this->value_type);
}

CVC4::Expr analysis::cvc_context::var(std::string name) {
  auto i_it = var_map.find(name);
  if(i_it != var_map.end()) return i_it->second;
  else {
    Expr i_exp = manager.mkVar(name, value_type);
    var_map[name] = i_exp;
    return i_exp;
  }
}

CVC4::Expr analysis::cvc_context::memory(size_t rev) {
  auto rev_it = mem_map.find(rev);
  if(rev_it != mem_map.end()) return rev_it->second;
  else {
    stringstream name;
    name << "m_" << rev;
    Expr mem = manager.mkVar(name.str(), mem_type);
    mem_map[rev] = mem;
    return mem;
  }
}

CVC4::Expr analysis::cvc_context::add(CVC4::Expr a, CVC4::Expr b) {
}

CVC4::Expr analysis::cvc_context::pack(CVC4::Expr expr) {
  return manager.mkExpr(kind::APPLY_CONSTRUCTOR, val_ctor, expr);
}

CVC4::Expr analysis::cvc_context::unpack(CVC4::Expr expr) {
  return manager.mkExpr(kind::APPLY_SELECTOR, val_sel, expr);
}
