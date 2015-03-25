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
#include <cppgdsl/rreil/expr/expr.h>
#include <cppgdsl/rreil/sexpr/sexpr.h>
#include <cppgdsl/rreil/linear/linear.h>

using namespace analysis::api;
using namespace gdsl::rreil;
using namespace summy;


num_var *converter::conv_id(id *id) {
  return new num_var(shared_copy(id));
}

//static num_linear *conv_id_lin(id *id) {
//  return new num_linear_term(conv_id(id));
//}

num_linear *converter::add(num_linear *a, vs_shared_t vs) {
  num_linear *result = NULL;
  num_visitor nv;
  nv._([&](num_linear_term *v) {
    num_var *nv = new num_var(v->get_var()->get_id());
    result = new num_linear_term(v->get_scale(), nv, add(v->get_next(), vs));
  });
  nv._([&](num_linear_vs *v) {
    result = new num_linear_vs(*vs + v->get_value_set());
  });
  a->accept(nv);
  return result;
}

num_linear *converter::add(num_linear *a, num_linear *b) {
  num_linear *result = NULL;
  num_visitor nv;
  nv._([&](num_linear_term *v) {
    num_var *nv = new num_var(v->get_var()->get_id());
    result = new num_linear_term(v->get_scale(), nv, add(v->get_next(), b));
  });
  nv._([&](num_linear_vs *v) {
    result = add(b, v->get_value_set());
  });
  a->accept(nv);
  return result;
}

num_linear *converter::conv_linear(linear *lin, int64_t scale) {
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
    result = transLE(shared_copy(l->get_var()->get_id()), l->get_var()->get_offset());
  });
  lin->accept(lv);
  return result;
}

analysis::api::num_expr *converter::conv_sexpr(sexpr *se) {
  num_expr *result = NULL;
  sexpr_visitor sv;
  sv._([&](arbitrary *x){
    result = new num_expr_lin(new num_linear_vs(value_set::top));
  });
  sv._([&](sexpr_cmp *x){
    result = new num_expr_lin(new num_linear_vs(value_set::top));
  });
  sv._([&](sexpr_lin *x){
    result = new num_expr_lin(conv_linear(x->get_lin()));
  });
  se->accept(sv);
  return result;
}

analysis::api::num_expr *converter::conv_expr(gdsl::rreil::expr *expr) {
  num_expr *result = NULL;
  expr_visitor ev;
  ev._([&](expr_binop *e) {
    num_linear *nl_opnd1 = conv_linear(e->get_opnd1());
    num_linear *nl_opnd2 = conv_linear(e->get_opnd2());
    auto rfo = [&](auto o) {
        result = new num_expr_bin(nl_opnd1, o, nl_opnd2);
    };
    switch(e->get_op()) {
      case BIN_MUL: {
        rfo(MUL);
        break;
      }
      case BIN_DIV:
      case BIN_DIVS: {
        rfo(DIV);
        break;
      }
      case BIN_MOD:
      case BIN_MODS: {
        rfo(MOD);
        break;
      }
      case BIN_SHL: {
        rfo(SHL);
        break;
      }
      case BIN_SHR: {
        rfo(SHR);
        break;
      }
      case BIN_SHRS: {
        rfo(SHRS);
        break;
      }
      case BIN_AND: {
        rfo(AND);
        break;
      }
      case BIN_OR: {
        rfo(OR);
        break;
      }
      case BIN_XOR: {
        rfo(XOR);
        break;
      }
    }
  });
  ev._([&](expr_ext *e) {
    /*
     * Todo: This is broken
     */
    num_linear *nl_opnd = conv_linear(e->get_opnd());
    result = new num_expr_lin(nl_opnd);
  });
  ev._([&](expr_sexpr *e) {
    result = conv_sexpr(e->get_inner());
  });
  expr->accept(ev);
  return result;
}

analysis::api::num_expr *analysis::api::converter::conv_expr(gdsl::rreil::linear *lin) {
  return new num_expr_lin(conv_linear(lin));
}

num_linear *converter::conv_linear(linear *lin) {
  return conv_linear(lin, 1);
}
