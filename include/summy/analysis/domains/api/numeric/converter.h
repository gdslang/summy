/*
 * converter.h
 *
 *  Created on: Mar 16, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include "num_expr.h"
#include <cppgdsl/rreil/expr/expr.h>

analysis::api::num_expr *conv_expr(gdsl::rreil::expr *expr);
