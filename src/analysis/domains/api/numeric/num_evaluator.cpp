/*
 * num_evaluator.cpp
 *
 *  Created on: Apr 11, 2015
 *      Author: jucs
 */

#include <summy/analysis/domains/numeric/num_evaluator.h>
#include <summy/analysis/domains/api/numeric/num_visitor.h>
#include <summy/value_set/value_set.h>
#include <include/summy/value_set/vs_finite.h>
#include <iostream>

using summy::value_set;
using summy::vs_finite;
using summy::vs_shared_t;

using namespace std;
using namespace analysis;
using namespace analysis::api;


summy::vs_shared_t analysis::num_evaluator::queryVal(api::num_linear *lin) {
  num_visitor nv;
  vs_shared_t result;
  nv._([&](num_linear_term *lt) {
    vs_shared_t scale = vs_finite::single(lt->get_scale());
    vs_shared_t id = query_var(lt->get_var());
    vs_shared_t next = queryVal(lt->get_next());
    result = *(*scale * id) + next;
  });
  nv._([&](num_linear_vs *lvs) {
    result = lvs->get_value_set();
  });
  lin->accept(nv);
  return result;
}

summy::vs_shared_t analysis::num_evaluator::queryVal(api::num_expr *exp) {
  num_visitor nv;
  vs_shared_t result;
  nv._([&](num_expr_cmp *cmp) {
    vs_shared_t opnd = queryVal(cmp->get_opnd());
    cout << "opnd: " << *opnd << endl;
    switch(cmp->get_op()) {
      case LE: {
        result = *opnd <= (int64_t)0;
        break;
      }
      case LT: {
        result = *opnd < (int64_t)0;
        break;
      }
      case GE: {
        result = *opnd >= (int64_t)0;
        break;
      }
      case GT: {
        result = *opnd > (int64_t)0;
        break;
      }
      case EQ: {
        result = *opnd == (int64_t)0;
        break;
      }
      case NEQ: {
        result = *opnd != (int64_t)0;
        break;
      }
    }
  });
  nv._([&](num_expr_lin *lin) {
    result = queryVal(lin->get_inner());
  });
  nv._([&](num_expr_bin *bin) {
    vs_shared_t opnd1 = queryVal(bin->get_opnd1());
    vs_shared_t opnd2 = queryVal(bin->get_opnd2());
    switch(bin->get_op()) {
      case MUL: {
        result = *opnd1 * opnd2;
        break;
      }
      case DIV: {
        result = *opnd1 / opnd2;
        break;
      }
      default: {
        result = value_set::top;
        break;
      }
    }
  });
  exp->accept(nv);
  cout << "queryVal result: " << *result << endl;
  return result;

}
