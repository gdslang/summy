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
typedef std::map<singleton_key_t, singleton_value_t, id_less_no_version> elements_t;

/**
 * Test two singleton_t instances for equality. The version of IDs (see ssa_id)
 * is ignored.
 */
bool singleton_equals(const singleton_t &a, const singleton_t &b);

typedef std::set<std::shared_ptr<gdsl::rreil::id>, id_less_no_version> id_set_t;

class adaptive_rd_elem : public lattice_elem {
private:
  bool contains_undef;
  const elements_t elements;
  size_t memory_rev;

public:
  const elements_t &get_elements() {
    return elements;
  }

  size_t get_memory_rev() {
    return memory_rev;
  }

  adaptive_rd_elem(elements_t elements, size_t memory_rev) :
      contains_undef(true), elements(elements), memory_rev(memory_rev) {
  }

  adaptive_rd_elem(bool contains_undef, elements_t elements, size_t memory_rev) :
      contains_undef(contains_undef), elements(elements), memory_rev(memory_rev) {
  }

  /*
   * Copy constructor
   */
  adaptive_rd_elem(adaptive_rd_elem &e) :
      lattice_elem(e), contains_undef(e.contains_undef), elements(e.elements), memory_rev(e.memory_rev) {
  }

  /**
   * Bottom constructor
   */
  adaptive_rd_elem() : contains_undef(false), memory_rev(0) {
  }


  virtual adaptive_rd_elem *lub(::analysis::lattice_elem *other, size_t current_node);
  virtual adaptive_rd_elem *add(std::vector<singleton_t> elements);
  virtual adaptive_rd_elem *remove(id_set_t elements);
  virtual adaptive_rd_elem *remove(std::function<bool(singleton_key_t, singleton_value_t)> pred);
  virtual adaptive_rd_elem *set_memory_rev(size_t memory_rev);

  virtual bool operator>=(::analysis::lattice_elem &other);

  virtual void put(std::ostream &out);
};

}
}
