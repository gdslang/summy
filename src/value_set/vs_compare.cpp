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

static bool less(vs_finite const *a, vs_finite const *b) {
  return a->get_elements() < b->get_elements();
}

static bool less(vs_open const *a, vs_open const *b) {
  if(a->get_limit() < b->get_limit())
    return true;
  return a->get_open_dir() < b->get_open_dir();
}

bool vs_total_less::operator ()(vs_shared_t a, vs_shared_t b) const {
  value_set_visitor vva;
  bool result;
  vva._([&](vs_finite const *_a) {
    value_set_visitor vvb;
    vvb._([&](vs_finite const *_b) {
      result = less(_a, _b);
    });
    vvb._([&](vs_open const *_b) {
      result = false;
    });
    vvb._([&](vs_top const *_b) {
      result = false;
    });
    b->accept(vvb);
  });
  vva._([&](vs_open const *_a) {
    value_set_visitor vvb;
    vvb._([&](vs_finite const *_b) {
      result = true;
    });
    vvb._([&](vs_open const *_b) {
      result = less(_a, _b);
    });
    vvb._([&](vs_top const *_b) {
      result = false;
    });
    b->accept(vvb);
  });
  vva._([&](vs_top const *_a) {
    value_set_visitor vvb;
    vvb._([&](vs_finite const *_b) {
      result = true;
    });
    vvb._([&](vs_open const *_b) {
      result = true;
    });
    vvb._([&](vs_top const *_b) {
      result = false;
    });
    b->accept(vvb);
  });
  a->accept(vva);
  return result;
}
