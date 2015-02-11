/*
 * vs_open.h
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/value_set/value_set_visitor.h>
#include "value_set.h"

namespace summy {

//enum vsb_type {
//  INFINITE, LIMITED
//};
//
//struct vs_boundary {
//  vsb_type type;
//  int64_t limit;
//
//  vs_boundary(vsb_type type, int64_t limit) :
//      type(type), limit(limit) {
//  }
//};

enum vs_open_dir {
  DOWNWARD, UPWARD
};

class vs_open: public value_set {
private:
  vs_open_dir open_dir;
  int64_t limit;

  void put(std::ostream &out);
public:
  vs_open(vs_open_dir open_dir, int64_t limit) : open_dir(open_dir), limit(limit) {
  }

  int64_t get_limit() const {
    return limit;
  }

  vs_open_dir get_open_dir() const {
    return open_dir;
  }

  void accept(value_set_visitor &v);
};

}
