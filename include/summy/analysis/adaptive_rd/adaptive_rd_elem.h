/*
 * rd_lattice_elem.h
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/lattice_elem.h>
#include <summy/analysis/subset_man.h>
#include <summy/analysis/util.h>
#include <set>
#include <map>
#include <tuple>
#include <ostream>
#include <memory>

namespace analysis {
namespace adaptive_rd {

typedef std::tuple<std::shared_ptr<gdsl::rreil::id>, size_t> singleton_t;
typedef std::tuple_element<0,singleton_t>::type singleton_key_t;
typedef std::tuple_element<1,singleton_t>::type singleton_value_t;
typedef std::map<singleton_key_t, singleton_value_t, id_less> elements_t;

//struct singleton_less {
//  bool operator()(singleton_t a, singleton_t b);
//};

typedef std::set<std::shared_ptr<gdsl::rreil::id>, id_less> id_set_t;

class adaptive_rd_elem : public lattice_elem {
private:
  bool contains_undef;
  const elements_t elements;

public:
  const elements_t &get_elements() {
    return elements;
  }

  adaptive_rd_elem(elements_t elements) : contains_undef(true), elements(elements) {
  }
  adaptive_rd_elem(bool contains_undef, elements_t elements) :
      contains_undef(contains_undef), elements(elements) {
  }

  /**
   * Bottom constructor
   */
  adaptive_rd_elem() : contains_undef(false) {
  }
  adaptive_rd_elem(adaptive_rd_elem &e) : lattice_elem(e), elements(e.elements) {
    this->contains_undef = e.contains_undef;
  }

  virtual adaptive_rd_elem *lub(::analysis::lattice_elem *other, size_t current_node);
  virtual adaptive_rd_elem *add(std::vector<singleton_t> elements);
  virtual adaptive_rd_elem *remove(id_set_t elements);
  virtual adaptive_rd_elem *remove(std::function<bool(singleton_key_t, singleton_value_t)> pred);

  virtual bool operator>=(::analysis::lattice_elem &other);

  virtual void put(std::ostream &out);
};

}
}
