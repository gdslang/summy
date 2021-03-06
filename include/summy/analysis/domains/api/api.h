/*
 * api.h
 *
 *  Created on: Mar 9, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include "numeric/num_expr.h"
#include "numeric/num_linear.h"
#include "numeric/num_var.h"
#include "numeric/num_visitor.h"
#include "numeric/converter.h"

#include "memory/mem_expr.h"
#include "memory/mem_visitor.h"

#include <memory>
#include <set>

namespace analysis {
namespace api {

var_ptr_set_t vars(num_expr *expr);

}
}
