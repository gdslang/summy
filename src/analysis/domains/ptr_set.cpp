/*
 * ptr_set.cpp
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/ptr_set.h>
#include <summy/value_set/vs_compare.h>

using namespace analysis;
using namespace summy;
using namespace std;

bool ptr::operator<(const ptr &other) const {
  if(id_less()(id, other.id))
    return true;
  else if(id_less()(other.id, id))
    return false;
  else
    return vs_total_less()(offset, other.offset);
}

bool analysis::ptr::operator==(const ptr &other) const {
  return !(*this < other) && !(other < *this);
}

std::ostream &analysis::operator<<(std::ostream &out, const ptr &_this) {
  out << "(" << *_this.id << " + " << *_this.offset << ")";
  return out;
}

// bool analysis::api::operator ==(const ptr_set_t &a, const ptr_set_t &b) {
//  return a == b;
//}

std::ostream &analysis::operator<<(std::ostream &out, ptr_set_t &_this) {
  out << "{";
  bool first = true;
  for(auto &ptr : _this) {
    if(first)
      first = false;
    else
      out << ", ";
    out << ptr;
  }
  out << "}";
  return out;
}
