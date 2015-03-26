/*
 * vs_top.cpp
 *
 *  Created on: Feb 12, 2015
 *      Author: Julian Kranz
 */

#include <summy/value_set/vs_top.h>
#include <summy/value_set/vs_open.h>
#include <summy/value_set/vs_finite.h>
#include <memory>

using namespace summy;
using namespace std;

void summy::vs_top::put(std::ostream &out) {
  out << "⊤";
}

vs_shared_t summy::vs_top::narrow(const vs_finite *vsf) const {
  return make_shared<vs_finite>(*vsf);
}

vs_shared_t summy::vs_top::narrow(const vs_open *vsf) const {
  return make_shared<vs_open>(*vsf);
}

vs_shared_t summy::vs_top::widen(const vs_finite *vsf) const {
  return value_set::top;
}

vs_shared_t summy::vs_top::widen(const vs_open *vsf) const {
  return value_set::top;
}

vs_shared_t summy::vs_top::add(const vs_finite *vs) const {
  return value_set::top;
}

vs_shared_t summy::vs_top::add(const vs_open *vs) const {
  return value_set::top;
}

vs_shared_t summy::vs_top::neg() const {
  return value_set::top;
}

vs_shared_t summy::vs_top::mul(const vs_finite *vs) const {
  return value_set::top;
}

vs_shared_t summy::vs_top::mul(const vs_open *vs) const {
  return value_set::top;
}

vs_shared_t summy::vs_top::div(const vs_finite *vs) const {
  return value_set::top;
}

vs_shared_t summy::vs_top::div(const vs_open *vs) const {
  return value_set::top;
}

vs_shared_t summy::vs_top::div(const vs_top *vs) const {
  return value_set::top;
}

bool summy::vs_top::smaller_equals(const vs_finite *vsf) const {
  return false;
}

bool summy::vs_top::smaller_equals(const vs_open *vsf) const {
  return false;
}

vs_shared_t summy::vs_top::meet(const vs_finite *vsf) const {
  return vsf->meet(this);
}

vs_shared_t summy::vs_top::meet(const vs_open *vsf) const {
  return vsf->meet(this);
}

vs_shared_t summy::vs_top::meet(const vs_top *vsf) const {
  return value_set::top;
}

vs_shared_t summy::vs_top::join(const vs_finite *vsf) const {
  return value_set::top;
}

vs_shared_t summy::vs_top::join(const vs_open *vsf) const {
  return value_set::top;
}

void summy::vs_top::accept(value_set_visitor &v) {
  v.visit(this);
}
