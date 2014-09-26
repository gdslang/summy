/*
 * rd_lattice_elem.h
 *
 *  Created on: Sep 25, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/lattice_elem.h>
#include <set>
#include <tuple>
#include <ostream>

namespace analysis {
namespace reaching_defs {

typedef std::tuple<size_t, gdsl::rreil::id*> singleton_t;

struct singleton_less {
  bool operator()(singleton_t a, singleton_t b);
};

typedef std::set<singleton_t, singleton_less> definitions_t;

class lattice_elem : public ::analysis::lattice_elem {
private:
  definitions_t ids;
public:
  lattice_elem(definitions_t ids) : ids(ids) {
  }
  virtual ::analysis::reaching_defs::lattice_elem *lub(::analysis::lattice_elem *other);
  virtual ::analysis::reaching_defs::lattice_elem *add(definitions_t ids);

  bool operator>(::analysis::lattice_elem &other);

  friend std::ostream &operator<< (std::ostream &out, lattice_elem &_this);
};

std::ostream &operator<<(std::ostream &out, lattice_elem &_this);

}
}
