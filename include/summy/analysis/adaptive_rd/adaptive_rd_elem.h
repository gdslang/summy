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
#include <tuple>
#include <ostream>
#include <memory>

namespace analysis {
namespace adaptive_rd {

typedef std::tuple<size_t, std::shared_ptr<gdsl::rreil::id>> singleton_t;

struct singleton_less {
  bool operator()(singleton_t a, singleton_t b);
};

typedef std::set<std::shared_ptr<gdsl::rreil::id>, id_less> id_set_t;

class adaptive_rd_elem : public lattice_elem {
public:
  typedef set_elem<singleton_t, singleton_less> rd_subset_man;
  typedef rd_subset_man::elements_t elements_t;
private:
  bool contains_undef;
  rd_subset_man eset;

  adaptive_rd_elem(bool contains_undef, rd_subset_man eset) :
      contains_undef(contains_undef), eset(eset) {
  }
public:
  adaptive_rd_elem(elements_t elements) : contains_undef(true), eset(elements) {
  }
  adaptive_rd_elem(bool contains_undef, elements_t elements) :
      contains_undef(contains_undef), eset(elements) {
  }

  /**
   * Bottom constructor
   */
  adaptive_rd_elem() : contains_undef(false), eset(elements_t { }) {
  }
  adaptive_rd_elem(adaptive_rd_elem &e) : lattice_elem(e), eset(e.eset) {
    this->contains_undef = e.contains_undef;
  }

  virtual adaptive_rd_elem *lub(::analysis::lattice_elem *other);
  virtual adaptive_rd_elem *add(elements_t elements);
  virtual adaptive_rd_elem *remove(elements_t elements);

  virtual adaptive_rd_elem *remove(id_set_t ids);

  virtual bool operator>=(::analysis::lattice_elem &other);

  virtual void put(std::ostream &out);
};

}
}
