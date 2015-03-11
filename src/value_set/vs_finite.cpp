/*
 * vs_finite.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */

#include <summy/value_set/vs_finite.h>
#include <summy/value_set/vs_open.h>
#include <bjutil/printer.h>
#include <algorithm>
#include <memory>

using namespace summy;
using namespace std;

void summy::vs_finite::put(std::ostream &out) {
  out << print(elements);
}

int64_t summy::vs_finite::min() const {
  return *elements.begin();
}

int64_t summy::vs_finite::max() const {
  return *elements.rbegin();
}

vs_shared_t summy::vs_finite::narrow(const vs_finite *vsf) const {
  return make_shared<vs_finite>(*vsf);
}

vs_shared_t summy::vs_finite::narrow(const vs_open *vsf) const {
  return value_set::bottom;
}

vs_shared_t summy::vs_finite::widen(const vs_finite *vsf) const {
  if(elements.empty())
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

bool summy::vs_finite::smaller_equals(const vs_finite *vsf) const {
  if(elements.empty())
    return true;
  if(vsf->elements.empty())
    return false;
  bool smallest_ge = min() >= vsf->min();
  bool greates_le = max() <= vsf->max();
  return smallest_ge && greates_le;
}

bool summy::vs_finite::smaller_equals(const vs_open *vsf) const {
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

void summy::vs_finite::accept(value_set_visitor &v) {
  v.visit(this);
}

vs_shared_t summy::vs_finite::single(int64_t value) {
  return make_shared<vs_finite>(vs_finite::elements_t { value });;
}

vs_shared_t const vs_finite::zero = make_shared<vs_finite>(elements_t { 0 });
size_t const vs_finite::max_growth = 100;
