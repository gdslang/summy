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
#include <summy/analysis/util.h>
#include <set>
//#include <tuple>
//#include <ostream>
#include <memory>

namespace analysis {
namespace liveness {

typedef std::shared_ptr<gdsl::rreil::id> singleton_t;

typedef id_less singleton_less;

typedef std::set<singleton_t, singleton_less> living_t;

class lv_elem : public ::analysis::lattice_elem {
private:
  living_t living;
public:
  lv_elem(living_t living) : living(living) {
  }
  virtual ~lv_elem();
  virtual lv_elem *lub(::analysis::lattice_elem *other);
  virtual lv_elem *add(living_t living);
  virtual lv_elem *remove(living_t living);;

  bool operator>(::analysis::lattice_elem &other);

//  friend std::ostream &operator<< (std::ostream &out, lattice_elem &_this);
};

//std::ostream &operator<<(std::ostream &out, lattice_elem &_this);

}
}
