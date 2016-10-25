/*
 * vs_top.h
 *
 *  Created on: Feb 12, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include "value_set.h"
#include "value_set_visitor.h"

namespace summy {

class vs_top: public value_set {
private:
  void put(std::ostream &out) const override;
public:
  vs_top() {
  }
  vs_shared_t narrow(vs_finite const *vsf) const override;
  vs_shared_t narrow(vs_open const *vsf) const override;

  vs_shared_t widen(vs_finite const *vsf) const override;
  vs_shared_t widen(vs_open const *vsf) const override;

  vs_shared_t add(vs_finite const *vs) const override;
  vs_shared_t add(vs_open const *vs) const override;

  vs_shared_t neg() const override;

  vs_shared_t mul(vs_finite const *vs) const override;
  vs_shared_t mul(vs_open const *vs) const override;

  vs_shared_t div(vs_finite const *vs) const override;
  vs_shared_t div(vs_open const *vs) const override;
  vs_shared_t div(vs_top const *vs) const override;

  vs_shared_t operator<=(int64_t v) const override;
  vs_shared_t operator<(int64_t v) const override;
  vs_shared_t operator==(int64_t v) const override;

  bool smaller_equals(vs_finite const *vsf) const override;
  bool smaller_equals(vs_open const *vsf) const override;

  vs_shared_t join(vs_finite const *vsf) const override;
  vs_shared_t join(vs_open const *vsf) const override;

  vs_shared_t meet(vs_finite const *vsf) const override;
  vs_shared_t meet(vs_open const *vsf) const override;
  vs_shared_t meet(vs_top const *vsf) const override;

  vs_shared_t with_sign_size(bool _unsigned, size_t size) const override;

  void accept(value_set_visitor &v) const override;
};

}
