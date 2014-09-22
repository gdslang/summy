/*
 * rreil_evaluator.cpp
 *
 *  Created on: Sep 22, 2014
 *      Author: Julian Kranz
 */

#include <summy/tools/rreil_evaluator.h>
#include <cppgdsl/rreil/rreil.h>

#include <functional>
#include <tuple>

using namespace std;
using namespace gdsl::rreil;

std::tuple<bool, int_t> rreil_evaluator::evaluate(class expr *expr) {
  function<tuple<bool, size_t>(linear*)> eval_linear;
  eval_linear = [&](linear *l) -> tuple<bool, size_t> {
    tuple<bool, size_t> result = make_tuple(false, 0);
    linear_visitor lv;
    lv._([&](lin_binop *lb) {
      bool evalable_opnd1;
      size_t value_opnd1;
      tie(evalable_opnd1, value_opnd1) = eval_linear(lb->get_opnd1());
      bool evalable_opnd2;
      size_t value_opnd2;
      tie(evalable_opnd2, value_opnd2) = eval_linear(lb->get_opnd2());
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
      tie(evalable_opnd, value_opnd) = eval_linear(ls->get_opnd());
      result = make_tuple(evalable_opnd, ls->get_const()*value_opnd);
    });
    lv._([&](lin_imm *lb) {
      result = make_tuple(true, lb->get_imm());
    });
    lv._([&](lin_var *lv) {
      if(variable_callback != NULL)
        result = variable_callback(lv->get_var());
    });
    l->accept(lv);
    return result;
  };
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
      result = eval_linear(sl->get_lin());
    });
    sex->get_inner()->accept(sexv);
  });
  expr->accept(ev);
  return result;
}
