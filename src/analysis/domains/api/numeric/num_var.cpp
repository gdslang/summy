/*
 * num_var.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/numeric/num_var.h>

std::ostream &analysis::api::operator <<(std::ostream &out, num_var &_this) {
  out << *_this.id;
  return out;
}

void analysis::api::num_vars::add(id_set_t const &ids) {
  this->ids.insert(ids.begin(), ids.end());
}
