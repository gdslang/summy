/*
 * memstate_util.cpp
 *
 *  Created on: May 27, 2015
 *      Author: jucs
 */

/*
 * managed temporary
 */
#include <summy/analysis/domains/memstate_util.h>

using namespace analysis;

analysis::managed_temporary::managed_temporary(memory_state_base &_this, api::num_var *var) :
    _this(_this), var(var) {
}

managed_temporary::~managed_temporary() {
  _this.child_state->kill( { var });
  delete var;
}

/*
 * field & relation
 */

std::ostream& analysis::operator <<(std::ostream &out, const field &_this) {
  out << "{" << *_this.num_id << ":" << _this.size << "}";
  return out;
}
