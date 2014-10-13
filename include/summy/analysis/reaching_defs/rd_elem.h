/*
 * rd_lattice_elem.h
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/lattice_elem.h>
#include <summy/analysis/set_elem.h>
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

typedef std::set<std::shared_ptr<gdsl::rreil::id>, id_less> id_set_t;

class rd_elem : public set_elem<singleton_t, singleton_less, rd_elem> {
private:
  bool contains_undef;
public:
  rd_elem(elements_t elements) : contains_undef(false), set_elem(elements) {
  }
  rd_elem(bool contains_undef, elements_t elements) :
      contains_undef(contains_undef), set_elem(elements) {
  }
  /**
   * Bottom constructor
   */
  rd_elem() : contains_undef(true), set_elem(elements_t { }) {
  }
  rd_elem(rd_elem &e) : set_elem(e) {
    this->contains_undef = e.contains_undef;
  }

  virtual rd_elem *lub(::analysis::lattice_elem *other);

  virtual rd_elem *add(elements_t elements)
  {
    return dynamic_cast<rd_elem*>(set_elem::add(elements)); //Stupid C++ :/
  }

  virtual rd_elem *remove(elements_t elements)
  {
    return dynamic_cast<rd_elem*>(set_elem::remove(elements)); //Stupid C++ :/
  }

  virtual rd_elem *remove(id_set_t ids);

  virtual bool operator>=(::analysis::lattice_elem &other);

  friend std::ostream &operator<< (std::ostream &out, rd_elem &_this);
};

std::ostream &operator<<(std::ostream &out, rd_elem &_this);

}
}
