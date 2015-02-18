/*
 * vs_open.h
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include "value_set.h"
#include <summy/value_set/value_set_visitor.h>

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
  vs_open(vs_open const &other) : open_dir(other.open_dir), limit(other.limit) {
  }

  int64_t get_limit() const {
    return limit;
  }

  vs_open_dir get_open_dir() const {
    return open_dir;
  }

  vs_shared_t narrow(vs_finite const *vsf) const;
  vs_shared_t narrow(vs_open const *vsf) const;

  vs_shared_t widen(vs_finite const *vsf) const;
  vs_shared_t widen(vs_open const *vsf) const;

  vs_shared_t add(vs_finite const *vs) const;
  vs_shared_t add(vs_open const *vs) const;

  vs_shared_t neg() const;

  vs_shared_t mul(vs_finite const *vs) const;
  vs_shared_t mul(vs_open const *vs) const;

  vs_shared_t div(vs_finite const *vs) const;
  vs_shared_t div(vs_open const *vs) const;
  vs_shared_t div(vs_top const *vs) const;

  bool smaller_equals(vs_finite const *vsf) const;
  bool smaller_equals(vs_open const *vsf) const;

  vs_shared_t join(vs_finite const *vsf) const;
  vs_shared_t join(vs_open const *vsf) const;

  void accept(value_set_visitor &v);

  bool one_sided() const;
};

}
