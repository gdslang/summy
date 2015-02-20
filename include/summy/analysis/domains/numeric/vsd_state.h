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
namespace value_sets {

typedef std::shared_ptr<gdsl::rreil::id> id_shared_t;
typedef std::tuple<id_shared_t, summy::vs_shared_t> singleton_t;
typedef std::tuple_element<0,singleton_t>::type singleton_key_t;
typedef std::tuple_element<1,singleton_t>::type singleton_value_t;
typedef std::map<singleton_key_t, singleton_value_t, id_less_no_version> elements_t;

/**
 * Value set domain state
 */
class vsd_state : public numeric_state {
private:
  const elements_t elements;

  summy::vs_shared_t lookup(id_shared_t id);
public:
  vsd_state(elements_t elements) : elements(elements) {
  }

  const elements_t &get_elements() const {
    return elements;
  }

  summy::vs_shared_t eval(api::num_linear *lin);
  summy::vs_shared_t eval(api::num_expr *exp);

  numeric_state *assign(api::num_var *lhs, api::num_expr *rhs);
};

}
}
