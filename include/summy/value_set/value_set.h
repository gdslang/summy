/*
 * value_set.h
 *
 *  Created on: Feb 11, 2015
 *      Author: jucs
 */

#pragma once
#include <iostream>
#include <memory>

namespace summy {

class value_set;
typedef std::shared_ptr<value_set> vs_shared_t;
class vs_top;
class vs_open;
class vs_finite;
class value_set_visitor;

class value_set {
private:
  virtual void put(std::ostream &out) = 0;
public:
  virtual ~value_set() {
  }
  friend std::ostream &operator<< (std::ostream &out, value_set &_this);
  virtual void accept(value_set_visitor &v) = 0;

  virtual bool smaller_equals(vs_finite const *vsf) const = 0;
  virtual bool smaller_equals(vs_open const *vsf) const = 0;
  bool smaller_equals(vs_top const *vsf) const;
  bool operator<=(vs_shared_t b);

  virtual vs_shared_t widen(vs_finite const *vsf) const = 0;
  virtual vs_shared_t widen(vs_open const *vsf) const = 0;
  vs_shared_t widen(vs_top const *vsf) const;
  static vs_shared_t widen(vs_shared_t a, vs_shared_t b);

  virtual vs_shared_t join(vs_finite const *vsf) const = 0;
  virtual vs_shared_t join(vs_open const *vsf) const = 0;
  vs_shared_t join(vs_top const *vsf) const;
  static vs_shared_t join(vs_shared_t a, vs_shared_t b);

  static vs_shared_t const top;
  static vs_shared_t const bottom;
};

std::ostream &operator<<(std::ostream &out, value_set &_this);

}
