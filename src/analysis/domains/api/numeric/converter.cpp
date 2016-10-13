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
using namespace std;

void analysis::api::expr_cmp_result_t::free() {
  delete primary;
  for(auto add : additional)
    delete add;
}

num_var *converter::conv_id(id const *id) {
  return new num_var(shared_copy(id));
}

//static num_linear *conv_id_lin(id *id) {
//  return new num_linear_term(conv_id(id));
//}

num_linear *converter::conv_linear(linear const *lin, int64_t scale) {
//  cout << "lin: " << *lin << ", scale: " << scale << endl;
  num_linear *result;
  linear_visitor lv;
  lv._([&](lin_binop const *l) {
    num_linear *opnd1 = conv_linear(&l->get_lhs(), scale);
    int64_t scale_opnd2;
    if(l->get_op() == BIN_LIN_ADD)
      scale_opnd2 = scale;
    else
      scale_opnd2 = -scale;
    num_linear *opnd2 = conv_linear(&l->get_rhs(), scale_opnd2);
    result = add(opnd1, opnd2);
    delete opnd1;
    delete opnd2;
  });
  lv._([&](lin_imm const *l) {
//    auto imm_vs = vs_finite::single(l->get_imm());
    auto imm_vs = vs_finite::single(scale*l->get_imm());
    result = new num_linear_vs(imm_vs);
  });
  lv._([&](lin_scale const *l) {
    result = conv_linear(&l->get_operand(), scale*l->get_const());
  });
  lv._([&](lin_var const *l) {
    num_linear *transLE_lin = transLE(shared_copy(&l->get_var().get_id()), l->get_var().get_offset(), size);
    result = mul(scale, transLE_lin);
    delete transLE_lin;
  });
  lin->accept(lv);
  return result;
}


expr_cmp_result_t analysis::api::converter::conv_expr_cmp(gdsl::rreil::sexpr_cmp const *se) {
  expr_cmp_result_t result;
  size_t sz_back = this->size;
  this->size = se->get_size();
  num_linear *opnd1 = conv_linear(&se->get_inner().get_lhs());
  num_linear *opnd2 = conv_linear(&se->get_inner().get_rhs());
  num_linear *opnd2_neg = opnd2->negate();
  switch(se->get_inner().get_op()) {
    case CMP_EQ: {
      result.primary = new num_expr_cmp(add(opnd1, opnd2_neg), EQ);
      break;
    }
    case CMP_NEQ: {
      result.primary = new num_expr_cmp(add(opnd1, opnd2_neg), NEQ);
      break;
    }
    case CMP_LEU: {
      result.additional.push_back(new num_expr_cmp(opnd1->copy(), GE));
      result.additional.push_back(new num_expr_cmp(opnd2->copy(), GE));
    }
    case CMP_LES: {
      result.primary = new num_expr_cmp(add(opnd1, opnd2_neg), LE);
      break;
    }
    case CMP_LTU: {
      result.additional.push_back(new num_expr_cmp(opnd1->copy(), GE));
      result.additional.push_back(new num_expr_cmp(opnd2->copy(), GE));
    }
    case CMP_LTS: {
      result.primary = new num_expr_cmp(add(opnd1, opnd2_neg), LT);
      break;
    }
  }
  this->size = sz_back;
  delete opnd1;
  delete opnd2;
  delete opnd2_neg;
  return result;
}

expr_cmp_result_t analysis::api::converter::conv_expr_cmp(gdsl::rreil::sexpr const *se) {
  expr_cmp_result_t result;
  sexpr_visitor sv;
  sv._([&](arbitrary const *x) {
    result.primary = new num_expr_cmp(new num_linear_vs(value_set::top), EQ);
  });
  sv._([&](sexpr_cmp const *x) {
    result = conv_expr_cmp(x);
  });
  sv._([&](sexpr_lin const *x) {
    size_t sz_back = this->size;
    this->size = 1;

    num_linear *x_lin = conv_linear(&x->get_lin());
    num_linear *minus_one = new num_linear_vs(vs_finite::single(-1));
    result.primary = new num_expr_cmp(add(x_lin, minus_one), EQ);
    delete x_lin;
    delete minus_one;

    this->size = sz_back;
  });
  se->accept(sv);
  return result;
}

analysis::api::num_expr *converter::conv_expr(sexpr const *se) {
  num_expr *result = NULL;
  sexpr_visitor sv;
  sv._([&](arbitrary const *x) {
    result = new num_expr_lin(new num_linear_vs(value_set::top));
  });
  sv._([&](sexpr_cmp const *x) {
    expr_cmp_result_t r_x = conv_expr_cmp(x);
    result = r_x.primary->copy();
    r_x.free();
  });
  sv._([&](sexpr_lin const *x) {
    result = new num_expr_lin(conv_linear(&x->get_lin()));
  });
  se->accept(sv);
  return result;
}

analysis::api::num_expr *converter::conv_expr(gdsl::rreil::expr const *expr) {
  num_expr *result = NULL;
  expr_visitor ev;
  ev._([&](expr_binop const *e) {
    num_linear *nl_opnd1 = conv_linear(&e->get_lhs());
    num_linear *nl_opnd2 = conv_linear(&e->get_rhs());
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
  ev._([&](expr_ext const *e) {
    size = e->get_fromsize();
    switch(e->get_op()) {
      case EXT_ZX: {
        num_linear *nl_opnd = conv_linear(&e->get_operand());
        result = new num_expr_lin(nl_opnd, sign_interp_t(UNSIGNED, e->get_fromsize()));
        break;
      }
      case EXT_SX: {
        num_linear *nl_opnd = conv_linear(&e->get_operand());
        result = new num_expr_lin(nl_opnd, sign_interp_t(SIGNED, e->get_fromsize()));
        break;
      }
    }
  });
  ev._([&](expr_sexpr const *e) {
    result = conv_expr(&e->get_inner());
  });
  expr->accept(ev);
  return result;
}

analysis::api::num_expr *analysis::api::converter::conv_expr(gdsl::rreil::linear const *lin) {
  return new num_expr_lin(conv_linear(lin));
}

num_linear *converter::conv_linear(linear const *lin) {
  return conv_linear(lin, 1);
}

num_linear *analysis::api::converter::mul(int64_t scale, num_linear *a) {
  num_linear *result = NULL;
  num_visitor nv;
  nv._([&](num_linear_term *v) {
    result = new num_linear_term(scale*v->get_scale(), v->get_var()->copy(), mul(scale, v->get_next()));
  });
  nv._([&](num_linear_vs *v) {
    result = new num_linear_vs(*v->get_value_set() * vs_finite::single(scale));
  });
  a->accept(nv);
  return result;
}

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
