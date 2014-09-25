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

namespace analysis {
namespace reaching_defs {

class lattice_elem : analysis::lattice_elem {
private:
  std::set<gdsl::rreil::id*> ids;
public:
  lattice_elem(std::set<gdsl::rreil::id*> ids) : ids(ids) {
  }
  virtual analysis::lattice_elem *lub(analysis::lattice_elem *other);
};


}
}
