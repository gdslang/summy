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

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

class value_set;
typedef std::shared_ptr<value_set const> vs_shared_t;
class vs_top;
class vs_open;
class vs_finite;
class value_set_visitor;

class value_set {
private:
  virtual void put(std::ostream &out) const = 0;
public:
  virtual ~value_set() {
  }
  friend std::ostream &operator<< (std::ostream &out, value_set const &_this);
  virtual void accept(value_set_visitor &v) const = 0;

  /*
   * Narrowing
   */
  virtual vs_shared_t narrow(vs_finite const *vsf) const = 0;
  virtual vs_shared_t narrow(vs_open const *vsf) const = 0;
  vs_shared_t narrow(vs_top const *vsf) const;
  static vs_shared_t narrow(vs_shared_t a, vs_shared_t b);

  /*
   * Widening
   */
  virtual vs_shared_t widen(vs_finite const *vsf) const = 0;
  virtual vs_shared_t widen(vs_open const *vsf) const = 0;
  vs_shared_t widen(vs_top const *vsf) const;
  static vs_shared_t widen(vs_shared_t a, vs_shared_t b);

  static vs_shared_t box(vs_shared_t a, vs_shared_t b);

  /*
   * Arithmetic
   */
  virtual vs_shared_t add(vs_finite const *vs) const = 0;
  virtual vs_shared_t add(vs_open const *vs) const = 0;
  vs_shared_t add(vs_top const *vs) const;
  vs_shared_t operator+(vs_shared_t b) const;
  vs_shared_t operator-(vs_shared_t b) const;

  virtual vs_shared_t neg() const = 0;
  vs_shared_t operator-() const;
  vs_shared_t operator!() const;

  virtual vs_shared_t mul(vs_finite const *vs) const = 0;
  virtual vs_shared_t mul(vs_open const *vs) const = 0;
  vs_shared_t mul(vs_top const *vs) const;
  vs_shared_t operator*(vs_shared_t b) const;

  virtual vs_shared_t div(vs_finite const *vs) const = 0;
  virtual vs_shared_t div(vs_open const *vs) const = 0;
  virtual vs_shared_t div(vs_top const *vs) const = 0;
  vs_shared_t operator/(vs_shared_t b) const;

  /*
   * Comparisons
   */
  virtual vs_shared_t operator<=(int64_t v) const = 0;
  virtual vs_shared_t operator<(int64_t v) const = 0;
  virtual vs_shared_t operator==(int64_t v) const = 0;
  vs_shared_t operator>=(int64_t v) const;
  vs_shared_t operator>(int64_t v) const;
  vs_shared_t operator!=(int64_t v) const;


  /*
   * Lattice operations
   */
  virtual bool smaller_equals(vs_finite const *vsf) const = 0;
  virtual bool smaller_equals(vs_open const *vsf) const = 0;
  bool smaller_equals(vs_top const *vsf) const;
  bool operator<=(value_set const *b) const;
  bool operator<=(vs_shared_t const b) const;

  bool operator==(vs_shared_t const b) const;

  virtual vs_shared_t join(vs_finite const *vsf) const = 0;
  virtual vs_shared_t join(vs_open const *vsf) const = 0;
  vs_shared_t join(vs_top const *vsf) const;
  static vs_shared_t join(vs_shared_t const a, vs_shared_t const b);

  virtual vs_shared_t meet(vs_finite const *vsf) const = 0;
  virtual vs_shared_t meet(vs_open const *vsf) const = 0;
  virtual vs_shared_t meet(vs_top const *vsf) const = 0;
  static vs_shared_t meet(vs_shared_t const a, vs_shared_t const b);

  /*
   * Extension
   */
  virtual vs_shared_t with_sign_size(bool _unsigned, size_t size) const = 0;

  static vs_shared_t const top;
  static vs_shared_t const bottom;
};

std::ostream &operator<<(std::ostream &out, value_set const &_this);

}

#include "vs_finite.h"
#include "vs_open.h"
#include "vs_top.h"
#include "vs_compare.h"
#include "value_set_visitor.h"
