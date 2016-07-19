/*
 * Copyright 2015-2016 Julian Kranz, Technical University of Munich
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * value_set.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */
#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_top.h>
#include <summy/value_set/vs_finite.h>
#include <summy/value_set/value_set_visitor.h>
#include <memory>

using namespace summy;
using namespace std;

std::ostream &summy::operator <<(std::ostream &out, value_set &_this) {
  _this.put(out);
  return out;
}

vs_shared_t value_set::narrow(const vs_top *vsf) const {
  return top;
}

vs_shared_t value_set::narrow(vs_shared_t a, vs_shared_t b) {
  value_set_visitor vs;
  vs_shared_t result;
  vs._([&] (vs_finite *v) {
    result = a->narrow(v);
  });
  vs._([&] (vs_open *v) {
    result = a->narrow(v);
  });
  vs._([&] (vs_top *v) {
    result = a->narrow(v);
  });
  b->accept(vs);
  return result;
}

vs_shared_t value_set::widen(const vs_top *vsf) const {
  return top;
}

vs_shared_t value_set::widen(vs_shared_t a, vs_shared_t b) {
  value_set_visitor vs;
  vs_shared_t result;
  vs._([&] (vs_finite *v) {
    result = a->widen(v);
  });
  vs._([&] (vs_open *v) {
    result = a->widen(v);
  });
  vs._([&] (vs_top *v) {
    result = a->widen(v);
  });
  b->accept(vs);
  return result;
}

vs_shared_t value_set::box(vs_shared_t a, vs_shared_t b) {
  if(*b <= a)
    return narrow(a, b);
  else
    return widen(a, b);
}

vs_shared_t summy::value_set::add(const vs_top *vs) const {
  return top;
}

vs_shared_t summy::value_set::operator +(vs_shared_t b) {
  value_set_visitor vs;
  vs_shared_t result;
  vs._([&] (vs_finite *v) {
    result = add(v);
  });
  vs._([&] (vs_open *v) {
    result = add(v);
  });
  vs._([&] (vs_top *v) {
    result = add(v);
  });
  b->accept(vs);
  return result;
}

vs_shared_t summy::value_set::operator -(vs_shared_t b) {
  return *this + (-*b);
}

vs_shared_t summy::value_set::operator -() const {
  return neg();
}

vs_shared_t summy::value_set::operator !() const {
  if(*this == vs_finite::_true)
    return vs_finite::_false;
  else if(*this == vs_finite::_false)
    return vs_finite::_true;
  else
    return vs_finite::_true_false;
}

vs_shared_t summy::value_set::mul(const vs_top *vs) const {
  return top;
}

vs_shared_t summy::value_set::operator *(vs_shared_t b) {
  value_set_visitor vs;
  vs_shared_t result;
  vs._([&] (vs_finite *v) {
    result = mul(v);
  });
  vs._([&] (vs_open *v) {
    result = mul(v);
  });
  vs._([&] (vs_top *v) {
    result = mul(v);
  });
  b->accept(vs);
  return result;
}

vs_shared_t summy::value_set::operator /(vs_shared_t b) {
  value_set_visitor vs;
  vs_shared_t result;
  vs._([&] (vs_finite *v) {
    result = div(v);
  });
  vs._([&] (vs_open *v) {
    result = div(v);
  });
  vs._([&] (vs_top *v) {
    result = div(v);
  });
  b->accept(vs);
  return result;
}

vs_shared_t summy::value_set::operator >=(int64_t v) const {
  return !*(*this < v);
}

vs_shared_t summy::value_set::operator >(int64_t v) const {
  return !*(*this <= v);
}

vs_shared_t summy::value_set::operator !=(int64_t v) const {
  return !*(*this == v);
}

bool summy::value_set::smaller_equals(const vs_top *vsf) const {
  return true;
}

bool summy::value_set::operator <=(value_set const *b) const {
  value_set_visitor vs;
  bool result;
  vs._([&] (vs_finite *v) {
    result = smaller_equals(v);
  });
  vs._([&] (vs_open *v) {
    result = smaller_equals(v);
  });
  vs._([&] (vs_top *v) {
    result = smaller_equals(v);
  });
  ((value_set*)b)->accept(vs);
  return result;
}

bool summy::value_set::operator <=(vs_shared_t const b) const {
  return operator <=(b.get());
}

bool summy::value_set::operator ==(vs_shared_t const b) const {
  return *this <= b && *b <= this;
}

vs_shared_t value_set::join(const vs_top *vsf) const {
  return top;
}

vs_shared_t value_set::join(vs_shared_t const a, vs_shared_t const b) {
  value_set_visitor vs;
  vs_shared_t result;
  vs._([&] (vs_finite *v) {
    result = a->join(v);
  });
  vs._([&] (vs_open *v) {
    result = a->join(v);
  });
  vs._([&] (vs_top *v) {
    result = a->join(v);
  });
  ((vs_shared_t)b)->accept(vs);
  return result;
}

vs_shared_t summy::value_set::meet(const vs_shared_t a, const vs_shared_t b) {
  value_set_visitor vs;
  vs_shared_t result;
  vs._([&] (vs_finite *v) {
    result = a->meet(v);
  });
  vs._([&] (vs_open *v) {
    result = a->meet(v);
  });
  vs._([&] (vs_top *v) {
    result = a->meet(v);
  });
  ((vs_shared_t)b)->accept(vs);
  return result;
}

vs_shared_t const value_set::top = make_shared<vs_top>();
vs_shared_t const value_set::bottom = make_shared<vs_finite>();
