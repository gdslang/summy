/*
 * mempath.cpp
 *
 *  Created on: Oct 29, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/mempath.h>
#include <string>

using namespace std;

bool mempath::step::operator<(const step &other) const {
  if(offset > other.offset)
    return true;
  else if(offset < other.offset)
    return false;
  else
    return size < other.size;
}

mempath::mempath(std::shared_ptr<gdsl::rreil::id> base, std::vector<step> path) : base(base), path(path) {
  if(path.size() < 1)
    throw string("Invalid path :-/");
}

int mempath::compare_to(const mempath &other) const {
  if(*base < *other.base)
    return -1;
  else if(*other.base < *base)
    return 1;
  else {
    if(path.size() < other.path.size())
      return -1;
    else if(path.size() > other.path.size())
      return 1;
    else
      for (int i = 0; i < path.size(); ++i) {
        if(path[i] < other.path[i])
          return -1;
        else if(other.path[i] < path[i])
          return 1;
      }
  }
  return 0;
}

bool mempath::operator<(const mempath &other) const {
  return compare_to(other) == -1;
}

bool mempath::operator==(const mempath &other) const {
  return compare_to(other) == 0;
}


std::ostream &operator<<(std::ostream &out, const mempath &_this) {
  out << *_this.base << "/" << _this.path[0].offset << ":" << _this.path[0].size;
  for (size_t i = 1; i < _this.path.size(); ++i)
    out << "->" <<  _this.path[i].offset << ":" << _this.path[i].size;
  return out;
}
