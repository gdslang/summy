/*
 * rd_lattice_elem.h
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/domain_state.h>

#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/subset_man.h>
#include <summy/analysis/util.h>
#include <set>
#include <tuple>
#include <ostream>
#include <memory>

namespace analysis {
namespace reaching_defs {

typedef std::tuple<size_t, std::shared_ptr<gdsl::rreil::id>> singleton_t;

struct singleton_less {
  bool operator()(singleton_t a, singleton_t b);
};

typedef std::set<std::shared_ptr<gdsl::rreil::id>, id_less_no_version> id_set_t;

class rd_state : public domain_state {
public:
  typedef set_elem<singleton_t, singleton_less> rd_subset_man;
  typedef rd_subset_man::elements_t elements_t;
private:
  bool contains_undef;
  rd_subset_man eset;

  rd_state(bool contains_undef, rd_subset_man eset) :
      contains_undef(contains_undef), eset(eset) {
  }
public:
  rd_state(elements_t elements) : contains_undef(true), eset(elements) {
  }
  rd_state(bool contains_undef, elements_t elements) :
      contains_undef(contains_undef), eset(elements) {
  }

  /**
   * Bottom constructor
   */
  rd_state() : contains_undef(false), eset(elements_t { }) {
  }
  rd_state(rd_state &e) : domain_state(e), eset(e.eset) {
    this->contains_undef = e.contains_undef;
  }

  virtual rd_state *join(::analysis::domain_state *other, size_t current_node);
  virtual rd_state *box(::analysis::domain_state *other, size_t current_node);
  virtual rd_state *add(elements_t elements);
  virtual rd_state *remove(elements_t elements);
  virtual rd_state *remove(std::function<bool(size_t, std::shared_ptr<gdsl::rreil::id>)> pred);

  virtual rd_state *remove(id_set_t ids);

  virtual bool operator>=(::analysis::domain_state &other);

  virtual void put(std::ostream &out);
};

}
}
