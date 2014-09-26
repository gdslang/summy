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
#include <ostream>

namespace analysis {
namespace reaching_defs {

class lattice_elem : public ::analysis::lattice_elem {
private:
  std::set<gdsl::rreil::id*> ids;
public:
  lattice_elem(std::set<gdsl::rreil::id*> ids) : ids(ids) {
  }
  virtual ::analysis::reaching_defs::lattice_elem *lub(::analysis::lattice_elem *other);
  virtual ::analysis::reaching_defs::lattice_elem *add(std::set<gdsl::rreil::id*> ids);

  bool operator>(::analysis::lattice_elem &other);

  friend std::ostream &operator<< (std::ostream &out, lattice_elem &_this);
};

std::ostream &operator<<(std::ostream &out, lattice_elem &_this);

}
}
