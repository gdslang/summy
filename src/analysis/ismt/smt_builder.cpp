/*
 * smt_builder.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/ismt/smt_builder.h>
#include <cppgdsl/rreil/rreil.h>

using namespace CVC4;
using namespace analysis;

void smt_builder::visit(gdsl::rreil::id *i) {
  auto i_str = i->to_string();
  auto &var_map = context.get_var_map();
}

//void analysis::smt_builder::visit(gdsl::rreil::variable *v) {
//}

void smt_builder::visit(gdsl::rreil::assign *a) {
}
