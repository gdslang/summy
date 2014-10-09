/*
 * lv_elem.h
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/lattice_elem.h>
#include <summy/analysis/set_elem.h>
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

//class lv_elem;
//typedef set_elem<singleton_t, singleton_less, lv_elem> lv_elem_base;
class lv_elem : public set_elem<singleton_t, singleton_less, lv_elem> {
public:
  lv_elem(elements_t elements) : set_elem(elements) {
  }

  virtual lv_elem *lub(::analysis::lattice_elem *other) {
    return dynamic_cast<lv_elem*>(set_elem::lub(other)); //Stupid C++ :/
  }

  virtual lv_elem *add(elements_t elements) {
    return dynamic_cast<lv_elem*>(set_elem::add(elements)); //Stupid C++ :/
  }

  virtual lv_elem *remove(elements_t elements) {
    return dynamic_cast<lv_elem*>(set_elem::remove(elements)); //Stupid C++ :/
  }

//  friend std::ostream &operator<< (std::ostream &out, lattice_elem &_this);
};

//std::ostream &operator<<(std::ostream &out, lattice_elem &_this);

}
}
