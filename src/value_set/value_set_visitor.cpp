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
 * value_set_visitor.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */
#include <summy/value_set/value_set_visitor.h>
#include <summy/value_set/vs_finite.h>
#include <summy/value_set/vs_open.h>
#include <summy/value_set/vs_top.h>
#include <string>
#include <assert.h>

using namespace summy;
using namespace std;

void value_set_visitor::visit(vs_finite const *v) {
  if(vs_finite_callback != NULL) vs_finite_callback(v);
  else _default(v);
}

void value_set_visitor::visit(vs_open const *v) {
  if(vs_open_callback != NULL) vs_open_callback(v);
  else _default(v);
}

void value_set_visitor::visit(vs_top const *v) {
  if(vs_top_callback != NULL) vs_top_callback(v);
  else _default(v);
}

void value_set_visitor::_default(value_set const *v) {
  if(default_callback != NULL) default_callback(v);
  else assert(ignore_default);
}
