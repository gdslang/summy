/*
 * cvc_context.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/ismt/cvc_context.h>
#include <cvc4/cvc4.h>

using namespace CVC4;

analysis::cvc_context::cvc_context() : smtEngine(&manager) {
  smtEngine.setOption("produce-models", SExpr("true"));
}
