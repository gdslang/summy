/*
 * num_expr.h
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#pragma once

namespace analysis {
namespace numeric {

class num_expr {

};

enum num_cmp_op {
  LE, GE, EQ, NEQ
};

class num_expr_cmp : public num_expr {

};

class num_exr_lin : public num_expr {

};

enum num_bin {
  MUL, DIV, MOD, SHL, SHR, AND, OR, XOR
};

class num_expr_bin : public num_expr {

};

}
}
