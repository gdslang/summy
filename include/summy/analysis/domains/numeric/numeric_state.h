/*
 * numeric.h
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domain_state.h>
#include <cppgdsl/rreil/id/id.h>
#include <memory>

namespace analysis {
namespace api {
class num_var;
class num_expr;
}

typedef std::shared_ptr<gdsl::rreil::id> id_shared_t;

class numeric_state : public domain_state {
public:
  virtual numeric_state *assign(api::num_var *lhs, api::num_expr *rhs) = 0;
};

}
