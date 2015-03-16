/*
 * converter.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/numeric/converter.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/rreil/shared_copy.h>
#include <summy/value_set/vs_finite.h>
#include <cppgdsl/rreil/expr/expr_visitor.h>
#include <cppgdsl/rreil/linear/linear_visitor.h>

using namespace analysis::api;
using namespace gdsl::rreil;
using namespace summy;

static num_var *conv_id(id *id) {
  return new num_var(shared_copy(id));
}

//static num_linear *conv_id(id *id) {
//  num_var *v = num_var(shared_copy(id));
//  return new num_linear_term(v);
//}

static num_linear *add(num_linear *a, vs_shared_t vs) {
  num_linear *result = NULL;
  num_visitor nv;
  nv._([&](num_linear_term *v) {
    num_var nv = new num_var(v->get_var()->get_id());
    result = new num_linear_term(v->get_scale(), nv, add(v->get_next(), vs));
  });
  nv._([&](num_linear_vs *v) {
    result = new num_linear_vs(*vs + v->get_value_set());
  });
  a->accept(nv);
  return result;

}

static num_linear *add(num_linear *a, num_linear *b) {
  num_linear *result = NULL;
  num_visitor nv;
  nv._([&](num_linear_term *v) {
    num_var nv = new num_var(v->get_var()->get_id());
    result = new num_linear_term(v->get_scale(), nv, add(v->get_next(), b));
  });
  nv._([&](num_linear_vs *v) {
    result = add(b, v->get_value_set());
  });
  a->accept(nv);
  return result;
}

static num_linear *conv_linear(linear *lin, int64_t scale) {
  num_linear *result;
  linear_visitor lv;
  lv._([&](lin_binop *l) {
    num_linear *opnd1 = conv_linear(l->get_opnd2(), scale);
    int64_t scale_opnd2;
    if(l->get_op() == BIN_LIN_ADD)
      scale_opnd2 = scale;
    else
      scale_opnd2 = -scale;
    num_linear *opnd2 = conv_linear(l->get_opnd1(), scale_opnd2);
    result = add(opnd1, opnd2);
    delete opnd1;
    delete opnd2;
  });
  lv._([&](lin_imm *l) {
//    auto imm_vs = vs_finite::single(l->get_imm());
    auto imm_vs = vs_finite::single(scale*l->get_imm());
    result = new num_linear_vs(imm_vs);
  });
  lv._([&](lin_scale *l) {
    result = conv_linear(l->get_opnd(), scale*l->get_const());
  });
  lv._([&](lin_var *l) {
    result = new num_linear_term(scale, conv_id(l->get_var()->get_id()));
  });
  lin->accept(lv);
  return result;
}

analysis::api::num_expr *conv_expr(gdsl::rreil::expr *expr) {
  num_expr *result = NULL;
  expr_visitor ev;
  ev._([&](expr_binop *e) {
    switch(e->get_op()) {
      case BIN_MUL: {
//        result = new num_expr_bin(conv_id(e->get))
        break;
      }
    }
  });
  ev._([&](expr_ext *e) {

  });
  ev._([&](expr_sexpr *e) {

  });
  expr->accept(ev);
}
