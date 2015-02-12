/*
 * vs_open.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */
#include <summy/value_set/vs_open.h>
#include <summy/value_set/vs_finite.h>

using namespace summy;

void summy::vs_open::put(std::ostream &out) {
  switch(open_dir) {
    case DOWNWARD: {
      out << "]-∞;" << limit << "]";
      break;
    }
    case UPWARD: {
      out << "[" << limit << ";∞]";
      break;
    }
  }
}

vs_shared_t summy::vs_open::join(const vs_finite *vsf) const {
  return vsf->join(this);
}

vs_shared_t summy::vs_open::join(const vs_open *vsf) const {
}

void summy::vs_open::accept(value_set_visitor &v) {
  v.visit(this);
}
