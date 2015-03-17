/*
 * vsd_state.h
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/domains/numeric/numeric_state.h>
#include <summy/value_set/value_set.h>
#include <summy/analysis/util.h>
#include <cppgdsl/rreil/id/id.h>
#include <map>

namespace analysis {
namespace api {
class num_linear;
}

namespace value_sets {

typedef std::tuple<id_shared_t, summy::vs_shared_t> singleton_t;
typedef std::tuple_element<0, singleton_t>::type singleton_key_t;
typedef std::tuple_element<1, singleton_t>::type singleton_value_t;
typedef std::map<singleton_key_t, singleton_value_t, id_less_no_version> elements_t;

/**
 * Value set domain state
 */
class vsd_state: public numeric_state {
private:
  bool bottom;
  const elements_t elements;
protected:
  void put(std::ostream &out) const;
public:
  vsd_state(elements_t elements) :
      bottom(false), elements(elements) {
  }
  vsd_state() :
      vsd_state(elements_t { }) {
  }
  vsd_state(bool bottom) :
      bottom(bottom), elements(elements_t { }) {
  }

  const elements_t &get_elements() const {
    return elements;
  }

  bool is_bottom() {
    return bottom;
  }

  summy::vs_shared_t eval(api::num_linear *lin);
  summy::vs_shared_t eval(api::num_expr *exp);

  summy::vs_shared_t lookup(id_shared_t id);

  bool operator>=(domain_state const &other) const;

  vsd_state *join(domain_state *other, size_t current_node);
  vsd_state *widen(domain_state *other, size_t current_node);
  vsd_state *narrow(domain_state *other, size_t current_node);
  vsd_state *box(domain_state *other, size_t current_node);

  vsd_state *assign(api::num_var *lhs, api::num_expr *rhs);
  numeric_state *assume(api::num_expr_cmp *cmp);
  numeric_state *assume(api::num_var *lhs, anaylsis::api::ptr_set_t aliases);
  numeric_state *kill(std::vector<api::num_var*> vars);
  numeric_state *equate_kill(num_var_pairs_t vars);
  numeric_state *fold(num_var_pairs_t vars);
};

}
}
