/*
 * vs_compare.cpp
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#include <summy/value_set/vs_compare.h>
#include <summy/value_set/value_set_visitor.h>
#include <summy/value_set/vs_finite.h>
#include <summy/value_set/vs_open.h>
#include <summy/value_set/vs_top.h>

using namespace summy;

static bool less(vs_finite *a, vs_finite *b) {
  return a->get_elements() < b->get_elements();
}

static bool less(vs_open *a, vs_open *b) {
  if(a->get_limit() < b->get_limit())
    return true;
  return a->get_open_dir() < b->get_open_dir();
}

bool vs_total_less::operator ()(vs_shared_t a, vs_shared_t b) const {
  value_set_visitor vva;
  bool result;
  vva._([&](vs_finite *_a) {
    value_set_visitor vvb;
    vvb._([&](vs_finite *_b) {
      result = less(_a, _b);
    });
    vvb._([&](vs_open *_b) {
      result = false;
    });
    vvb._([&](vs_top *_b) {
      result = false;
    });
    b->accept(vvb);
  });
  vva._([&](vs_open *_a) {
    value_set_visitor vvb;
    vvb._([&](vs_finite *_b) {
      result = true;
    });
    vvb._([&](vs_open *_b) {
      result = less(_a, _b);
    });
    vvb._([&](vs_top *_b) {
      result = false;
    });
    b->accept(vvb);
  });
  vva._([&](vs_top *_a) {
    value_set_visitor vvb;
    vvb._([&](vs_finite *_b) {
      result = true;
    });
    vvb._([&](vs_open *_b) {
      result = true;
    });
    vvb._([&](vs_top *_b) {
      result = false;
    });
    b->accept(vvb);
  });
  a->accept(vva);
  return result;
}
