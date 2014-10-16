/*
 * lv_elem.h
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/lattice_elem.h>
#include <summy/analysis/liveness/lv_elem.h>
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
typedef std::map<singleton_key_t, singleton_value_t, id_less> elements_t;

class lv_elem: public lattice_elem {
private:
  const elements_t elements;
public:
  lv_elem(elements_t elements) :
      elements(elements) {
  }

  virtual lv_elem *lub(::analysis::lattice_elem *other);
  virtual lv_elem *add(std::vector<singleton_t> elements);
  virtual lv_elem *remove(std::vector<singleton_t> elements);

  /*
   * Check whether the lattice element contains at least one live
   * bit of the given singleton
   */
  bool contains_bit(singleton_t s);
  virtual bool operator>=(::analysis::lattice_elem &other);

  virtual void put(std::ostream &out);
};

}
}
