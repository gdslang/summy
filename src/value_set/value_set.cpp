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

bool summy::value_set::smaller_equals(const vs_top *vsf) const {
  return true;
}

bool summy::value_set::operator <=(vs_shared_t b) {
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
  b->accept(vs);
  return result;
}

vs_shared_t value_set::box(vs_shared_t a, vs_shared_t b) {
  if(*b <= a)
    return narrow(a, b);
  else
    return widen(a, b);
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

vs_shared_t value_set::join(const vs_top *vsf) const {
  return top;
}

vs_shared_t value_set::join(vs_shared_t a, vs_shared_t b) {
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
  b->accept(vs);
  return result;
}

vs_shared_t const value_set::top = make_shared<vs_top>();
vs_shared_t const value_set::bottom = make_shared<vs_finite>();
