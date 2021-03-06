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

analysis::cvc_context::cvc_context(bool unsat_cores) : smtEngine(&manager) {
  smtEngine.setOption("produce-models", SExpr("true"));
  smtEngine.setOption("incremental", SExpr("true"));
  if(unsat_cores) {
    smtEngine.setOption("produce-unsat-cores", SExpr("true"));
//    smtEngine.setOption("tear-down-incremental", SExpr("true"));
  }

  mem_type = manager.mkArrayType(manager.mkBitVectorType(61), manager.mkBitVectorType(64));
}

CVC4::Expr analysis::cvc_context::var(std::string name) {
  auto i_it = var_map.find(name);
  if(i_it != var_map.end()) return i_it->second;
  else {
    Expr i_exp = manager.mkVar(name, manager.mkBitVectorType(64));
    var_map[name] = i_exp;
    return i_exp;
  }
}

CVC4::Expr analysis::cvc_context::memory(memory_map_t &m, string base, size_t rev) {
  auto rev_it = m.find(rev);
  if(rev_it != m.end()) return rev_it->second;
  else {
    stringstream name;
    name << base << rev;
    Expr mem = manager.mkVar(name.str(), mem_type);
    m[rev] = mem;
    return mem;
  }
}

//CVC4::Expr analysis::cvc_context::var_def(std::string name) {
//  return var(name + "_def");
//}

CVC4::Expr analysis::cvc_context::memory(size_t rev) {
  return memory(mem_map, "m_", rev);
}

CVC4::Expr analysis::cvc_context::memory_def(size_t rev) {
  return memory(mem_def_map, "m_def_", rev);
}
