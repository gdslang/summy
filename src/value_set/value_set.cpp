/*
 * value_set.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */
#include <summy/value_set/value_set.h>

std::ostream &summy::operator <<(std::ostream &out, value_set &_this) {
  _this.put(out);
  return out;
}
