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

vs_shared_t summy::vs_finite::join(const vs_finite *vsf) const {
  elements_t elements_new;
  set_intersection(elements.begin(), elements.end(), vsf->elements.begin(), vsf->elements.end(),
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
