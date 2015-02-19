/*
 * num_var.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/numeric/api/num_var.h>

std::ostream &analysis::numeric::operator <<(std::ostream &out, num_var &_this) {
  out << *_this.id;
  return out;
}
