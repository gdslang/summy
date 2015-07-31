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
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/sm_id.h>
#include <summy/rreil/id/special_ptr.h>
#include <iostream>
#include <experimental/optional>

using summy::value_set;
using summy::vs_finite;
using summy::vs_shared_t;
using std::experimental::optional;

using namespace std;
using namespace analysis;
using namespace analysis::api;
using namespace summy::rreil;

summy::vs_shared_t analysis::num_evaluator::queryVal(api::num_var *var) {
  optional<vs_shared_t> id_value;
  summy::rreil::id_visitor idv;
  idv._([&](sm_id *sid) {
    id_value = vs_finite::single((int64_t)sid->get_address());
  });
  idv._([&](special_ptr *sp) {
    switch(sp->get_kind()) {
      case NULL_PTR: {
        id_value = vs_finite::single(0);
        break;
      }
      case BAD_PTR: {
        id_value = value_set::top;
        break;
      }
    }
  });
  var->get_id()->accept(idv);

  if(!id_value)
    id_value = query_var(var);

  return id_value.value();
}

summy::vs_shared_t analysis::num_evaluator::queryVal(api::num_linear *lin) {
  num_visitor nv;
  vs_shared_t result;
  nv._([&](num_linear_term *lt) {
    vs_shared_t scale = vs_finite::single(lt->get_scale());

    vs_shared_t id_value = queryVal(lt->get_var());

    vs_shared_t next = queryVal(lt->get_next());
    result = *(*scale * id_value) + next;
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
  return result;

}
