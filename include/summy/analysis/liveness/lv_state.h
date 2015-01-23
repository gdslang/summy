/*
 * lv_elem.h
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/domain_state.h>

#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/subset_man.h>
#include <summy/analysis/util.h>
#include <set>
#include <map>
#include <tuple>
#include <memory>

namespace analysis {
namespace liveness {

typedef std::tuple<std::shared_ptr<gdsl::rreil::id>, unsigned long long int> singleton_t;
typedef std::tuple_element<0,singleton_t>::type singleton_key_t;
typedef std::tuple_element<1,singleton_t>::type singleton_value_t;
typedef std::map<singleton_key_t, singleton_value_t, id_less_no_version> elements_t;

class lv_state: public domain_state {
private:
  const elements_t elements;
public:
  lv_state(elements_t elements) :
      elements(elements) {
  }

  virtual lv_state *join(::analysis::domain_state *other, size_t current_node);
  virtual lv_state *add(std::vector<singleton_t> elements);
  virtual lv_state *remove(std::vector<singleton_t> elements);

  /*
   * Check whether the lattice element contains at least one live
   * bit of the given singleton
   */
  bool contains_bit(singleton_t s);
  virtual bool operator>=(::analysis::domain_state &other);

  virtual void put(std::ostream &out);
};

}
}
