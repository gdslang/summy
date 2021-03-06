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
 * vs_finite.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <summy/value_set/vs_finite.h>
#include <summy/value_set/vs_open.h>
#include <bjutil/printer.h>
#include <algorithm>
#include <memory>

using namespace summy;
using namespace std;

void summy::vs_finite::put(std::ostream &out) const {
  out << print(elements);
}

int64_t summy::vs_finite::min() const {
  if(elements.empty())
    throw string("summy::vs_finite::min()");
  return *elements.begin();
}

int64_t summy::vs_finite::max() const {
  if(elements.empty())
    throw string("summy::vs_finite::max()");
  return *elements.rbegin();
}

bool summy::vs_finite::is_bottom() const {
  return elements.empty();
}

bool summy::vs_finite::is_singleton() const {
  return elements.size() == 1;
}

vs_shared_t summy::vs_finite::narrow(const vs_finite *vsf) const {
  return make_shared<vs_finite>(*vsf);
}

vs_shared_t summy::vs_finite::narrow(const vs_open *vsf) const {
  return value_set::bottom;
}

vs_shared_t summy::vs_finite::widen(const vs_finite *vsf) const {
  if(is_bottom())
    return make_shared<vs_finite>(*vsf);
  if(elements == vsf->get_elements())
    return make_shared<vs_finite>(*this);
  if(min() <= vsf->min())
    return make_shared<vs_open>(UPWARD, min());
  if(max() >= vsf->max())
    return make_shared<vs_open>(DOWNWARD, max());
  return value_set::top;
}

vs_shared_t summy::vs_finite::widen(const vs_open *vsf) const {
  if(is_bottom())
    return make_shared<vs_open>(*vsf);
  switch(vsf->get_open_dir()) {
    case DOWNWARD: {
      if(vsf->get_limit() >= max())
        return make_shared<vs_open>(*vsf);
      break;
    }
    case UPWARD: {
      if(vsf->get_limit() <= min())
        return make_shared<vs_open>(*vsf);
      break;
    }
  }
  return value_set::top;
}

vs_shared_t summy::vs_finite::add(const vs_finite *vs) const {
  elements_t re;
  for(auto e1 : elements)
    for(auto e2 : vs->elements)
      re.insert(e1 + e2);
  return make_shared<vs_finite>(re);
}

vs_shared_t summy::vs_finite::add(const vs_open *vs) const {
  if(is_bottom())
    return value_set::bottom;
  switch(vs->get_open_dir()) {
    case DOWNWARD: {
      return make_shared<vs_open>(DOWNWARD, max() + vs->get_limit());
    }
    case UPWARD: {
      return make_shared<vs_open>(UPWARD, min() + vs->get_limit());
    }
  }
}

vs_shared_t summy::vs_finite::neg() const {
  elements_t re;
  for(auto e : elements)
    re.insert(-e);
  return make_shared<vs_finite>(re);
}

vs_shared_t summy::vs_finite::mul(const vs_finite *vs) const {
  elements_t re;
  for(auto e1 : elements)
    for(auto e2 : vs->elements)
      re.insert(e1 * e2);
  return make_shared<vs_finite>(re);
}

vs_shared_t summy::vs_finite::mul(const vs_open *vs) const {
  if(is_bottom())
    return value_set::bottom;
  if(min() == 0 && max() == 0)
    return zero;
  if(sgn(max())*sgn(min()) < 0)
    return top;
  if(sgn(min()) + sgn(max()) < 0)
    return *(-(*this)) * (-(*vs));
  switch(vs->get_open_dir()) {
    case DOWNWARD: {
      if(vs->get_limit() < 0)
        return make_shared<vs_open>(DOWNWARD, min() * vs->get_limit());
      else
        return make_shared<vs_open>(DOWNWARD, max() * vs->get_limit());
    }
    case UPWARD: {
      if(vs->get_limit() < 0)
        return make_shared<vs_open>(UPWARD, max() * vs->get_limit());
      else
        return make_shared<vs_open>(UPWARD, min() * vs->get_limit());
    }
  }
}

vs_shared_t summy::vs_finite::div(const vs_finite *vs) const {
  elements_t re;
  for(auto e1 : elements)
    for(auto e2 : vs->elements) {
      if(e2 == 0)
        re.insert(0);
      else if(e1 % e2 == 0)
        re.insert(e1 / e2);
    }
  return make_shared<vs_finite>(re);
}

vs_shared_t summy::vs_finite::div(const vs_open *vs) const {
  if(is_bottom())
    return value_set::bottom;
  if(min() < -max_growth && max() > max_growth)
    return top;
  int64_t sign;
  bool one_sided = vs->one_sided();
  if(one_sided) sign = vs->get_open_dir() == DOWNWARD ? -1 : 1;
  else sign = 1;
  elements_t er;
  if(min() < 0) for(int64_t i = min(); i < 0; i++) {
    er.insert(sign * i);
    if(!one_sided) er.insert(-i);
  }
  if(max() > 0) for(int64_t i = 1; i <= max(); i++) {
    er.insert(sign * i);
    if(!one_sided) er.insert(-i);
  }
  er.insert(0);
  return make_shared<vs_finite>(er);
}

vs_shared_t summy::vs_finite::div(const vs_top *vs) const {
  if(is_bottom())
    return value_set::bottom;
  if(min() < -max_growth && max() > max_growth)
    return top;
  elements_t er;
  if(min() < 0) for(int64_t i = min(); i < 0; i++) {
    er.insert(i);
    er.insert(-i);
  }
  if(max() > 0) for(int64_t i = 1; i <= max(); i++) {
    er.insert(i);
    er.insert(-i);
  }
  er.insert(0);
  return make_shared<vs_finite>(er);
}

vs_shared_t summy::vs_finite::operator <=(int64_t v) const {
  if(is_bottom())
    return value_set::bottom;
  if(max() <= v)
    return _true;
  else if(min() > v)
    return _false;
  else
    return _true_false;
}

vs_shared_t summy::vs_finite::operator <(int64_t v) const {
  if(is_bottom())
    return value_set::bottom;
  if(max() < v)
    return _true;
  else if(min() >= v)
    return _false;
  else
    return _true_false;
}

vs_shared_t summy::vs_finite::operator ==(int64_t v) const {
  if(is_bottom())
    return value_set::bottom;;
  if((min() == max()) && (min() == v))
    return _true;
  else if(elements.find(v) == elements.end())
    return _false;
  else
    return _true_false;
}

bool summy::vs_finite::smaller_equals(const vs_finite *vsf) const {
  if(is_bottom())
    return true;
  if(vsf->is_bottom())
    return false;
  bool smallest_ge = min() >= vsf->min();
  bool greates_le = max() <= vsf->max();
  return smallest_ge && greates_le;
}

bool summy::vs_finite::smaller_equals(const vs_open *vsf) const {
  if(is_bottom())
    return true;
  switch(vsf->get_open_dir()) {
    case DOWNWARD: {
      bool greatest_le = max() <= vsf->get_limit();
      return greatest_le;
    }
    case UPWARD: {
      bool smallest_ge = min() >= vsf->get_limit();
      return smallest_ge;
    }
  }
}

vs_shared_t summy::vs_finite::join(const vs_finite *vsf) const {
  elements_t elements_new;
  set_union(elements.begin(), elements.end(), vsf->elements.begin(), vsf->elements.end(),
      inserter(elements_new, elements_new.begin()));
  return make_shared<vs_finite>(elements_new);
}

vs_shared_t summy::vs_finite::join(const vs_open *vsf) const {
  if(is_bottom())
    return make_shared<vs_open>(*vsf);
  switch(vsf->get_open_dir()) {
    case DOWNWARD: {
      return make_shared<vs_open>(DOWNWARD, std::max(vsf->get_limit(), max()));
      break;
    }
    case UPWARD: {
      return make_shared<vs_open>(UPWARD, std::min(vsf->get_limit(), min()));
      break;
    }
  }
}

vs_shared_t summy::vs_finite::meet(const vs_finite *vsf) const {
  elements_t elements_new;
  set_intersection(elements.begin(), elements.end(), vsf->elements.begin(), vsf->elements.end(),
      inserter(elements_new, elements_new.begin()));
  return make_shared<vs_finite>(elements_new);
}

vs_shared_t summy::vs_finite::meet(const vs_open *vsf) const {
  elements_t elements_new;
  int64_t limit = vsf->get_limit();
  switch(vsf->get_open_dir()) {
    case DOWNWARD: {
      for(auto it = elements.begin(); it != elements.end(); it++) {
        int64_t next = *it;
        if(next > limit)
          break;
        elements_new.insert(next);
      }
      break;
    }
    case UPWARD: {
      for(auto it = elements.rbegin(); it != elements.rend(); it++) {
        int64_t next = *it;
        if(next < limit)
          break;
        elements_new.insert(next);
      }
      break;
    }
  }
  return make_shared<vs_finite>(elements_new);
}

vs_shared_t summy::vs_finite::meet(const vs_top *vsf) const {
  return make_shared<vs_finite>(*this);
}

vs_shared_t summy::vs_finite::with_sign_size(bool _unsigned, size_t size) const {
  if(size > 64)
    return value_set::top;
  else if(size < 64 && size > 0) {
    assert((size & (size - 1)) == 0);
    uint64_t mask = ((uint64_t)1 << (uint64_t)size) - 1;

    elements_t elements_new;
    for(int64_t element : elements) {
      if(!_unsigned && (element & (1 << (size - 1)))) {
        elements_new.insert(element | ~mask);
      } else
        elements_new.insert(element & mask);
    }
    return make_shared<vs_finite>(elements_new);
  } else
    return make_shared<vs_finite>(*this);
}

void summy::vs_finite::accept(value_set_visitor &v) const {
  v.visit(this);
}

vs_shared_t summy::vs_finite::single(int64_t value) {
  return make_shared<vs_finite>(vs_finite::elements_t { value });
}

vs_shared_t const vs_finite::_true = single(1);
vs_shared_t const vs_finite::_false = single(0);
vs_shared_t const vs_finite::_true_false = value_set::join(_true, _false);

vs_shared_t const vs_finite::zero = make_shared<vs_finite>(elements_t { 0 });
size_t const vs_finite::max_growth = 512;
