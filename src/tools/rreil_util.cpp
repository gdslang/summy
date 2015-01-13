/*
 * rreil_evaluator.cpp
 *
 *  Created on: Sep 22, 2014
 *      Author: Julian Kranz
 */

#include <summy/tools/rreil_util.h>

#include <cppgdsl/rreil/rreil.h>
#include <cppgdsl/rreil/statement/statement_visitor.h>
#include <cppgdsl/rreil/expr/expr_visitor.h>
#include <cppgdsl/rreil/sexpr/sexpr_visitor.h>
#include <summy/rreil/visitor.h>

#include <functional>
#include <tuple>

using namespace std;
using namespace std::placeholders;
using namespace gdsl::rreil;
namespace sr = summy::rreil;

std::tuple<bool, int_t> rreil_evaluator::evaluate(linear *lin) {
  tuple<bool, size_t> result = make_tuple(false, 0);
  linear_visitor lv;
  lv._([&](lin_binop *lb) {
    bool evalable_opnd1;
    size_t value_opnd1;
    tie(evalable_opnd1, value_opnd1) = evaluate(lb->get_opnd1());
    bool evalable_opnd2;
    size_t value_opnd2;
    tie(evalable_opnd2, value_opnd2) = evaluate(lb->get_opnd2());
    bool evalable = evalable_opnd1 && evalable_opnd2;
    switch(lb->get_op()) {
      case BIN_LIN_ADD: {
        result = make_tuple(evalable, value_opnd1 + value_opnd2);
        break;
      }
      case BIN_LIN_SUB: {
        result = make_tuple(evalable, value_opnd1 - value_opnd2);
        break;
      }
    }
  });
  lv._([&](lin_scale *ls) {
    bool evalable_opnd;
    size_t value_opnd;
    tie(evalable_opnd, value_opnd) = evaluate(ls->get_opnd());
    result = make_tuple(evalable_opnd, ls->get_const()*value_opnd);
  });
  lv._([&](lin_imm *lb) {
    result = make_tuple(true, lb->get_imm());
  });
  lv._([&](lin_var *lv) {
    if(variable_callback != NULL)
      result = variable_callback(lv->get_var());
  });
  lin->accept(lv);
  return result;
}

std::tuple<bool, int_t> rreil_evaluator::evaluate(class expr *expr) {
  tuple<bool, size_t> result = make_tuple(false, 0);
  expr_visitor ev;
  ev._([&](expr_binop *eb) {
    /*
     * Todo ;-)
     */
//    switch(eb->get_op()) {
//    }
  });
  ev._([&](expr_ext *ee) {
    /*
     * Todo :-(
     */
  });
  ev._([&](expr_sexpr *sex) {
    sexpr_visitor sexv;
    sexv._([&](sexpr_lin *sl) {
      result = evaluate(sl->get_lin());
    });
    sex->get_inner()->accept(sexv);
  });
  expr->accept(ev);
  return result;
}

bool rreil_prop::is_ip(gdsl::rreil::variable *v) {
  bool is_ip = false;
  id_visitor iv;
  iv._([&] (arch_id *ai) {
    if(ai->get_name() == "IP")
      is_ip = true;
  });
  v->get_id()->accept(iv);
  return is_ip;
}

int_t rreil_prop::size_of_rhs(gdsl::rreil::assign *a) {
  int_t size = a->get_size();
  expr_visitor ev;
  ev._([&](expr_sexpr *s){
    sexpr_visitor sv;
    sv._([&](sexpr_cmp *cp){
      size = cp->get_size();
    });
    s->get_inner()->accept(sv);
  });
  ev._([&](expr_ext *e) {
    size = e->get_fromsize();
  });
  a->get_rhs()->accept(ev);
  return size;
};
