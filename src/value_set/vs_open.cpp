/*
 * vs_open.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */
#include <summy/value_set/vs_open.h>

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

void summy::vs_open::accept(value_set_visitor &v) {
  v.visit(this);
}
