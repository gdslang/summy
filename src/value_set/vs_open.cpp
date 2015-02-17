/*
 * vs_open.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: Julian Kranz
 */
#include <summy/value_set/vs_open.h>
#include <summy/value_set/vs_finite.h>
#include <algorithm>

using namespace summy;
using namespace std;

void summy::vs_open::put(std::ostream &out) {
  switch(open_dir) {
    case DOWNWARD: {
      out << "]-∞;" << limit << "]";
      break;
    }
    case UPWARD: {
      out << "[" << limit << ";∞]";
      break;
    }
  }
}

bool summy::vs_open::smaller_equals(const vs_finite *vsf) const {
  return false;
}

bool summy::vs_open::smaller_equals(const vs_open *vsf) const {
  switch(open_dir) {
    case DOWNWARD: {
      switch(vsf->open_dir) {
        case DOWNWARD: {
          return limit <= vsf->get_limit();
        }
        case UPWARD: {
            return false;
        }
      }
      break;
    }
    case UPWARD: {
      switch(vsf->open_dir) {
        case DOWNWARD: {
          return false;
        }
        case UPWARD: {
          return limit >= vsf->get_limit();
        }
      }
    }
  }
}

vs_shared_t summy::vs_open::widen(const vs_finite *vsf) const {
  switch(open_dir) {
    case DOWNWARD: {
      if(vsf->max() <= limit)
        return make_shared<vs_open>(*this);
      break;
    }
    case UPWARD: {
      if(vsf->min() >= limit)
        return make_shared<vs_open>(*this);
      break;
    }
  }
  return value_set::top;
}

vs_shared_t summy::vs_open::widen(const vs_open *vsf) const {
  switch(open_dir) {
    case DOWNWARD: {
      switch(vsf->open_dir) {
        case DOWNWARD: {
          if(get_limit() >= vsf->get_limit())
            return make_shared<vs_open>(*this);
          break;
        }
        default: break;
      }
      break;
    }
    case UPWARD: {
      switch(vsf->open_dir) {
        case UPWARD: {
          if(get_limit() <= vsf->get_limit())
            return make_shared<vs_open>(*this);
          break;
        }
        default: break;
      }
      break;
    }
  }
  return value_set::top;
}

vs_shared_t summy::vs_open::join(const vs_finite *vsf) const {
  return vsf->join(this);
}

vs_shared_t summy::vs_open::join(const vs_open *vsf) const {
  switch(open_dir) {
    case DOWNWARD: {
      switch(vsf->open_dir) {
        case DOWNWARD: {
          return make_shared<vs_open>(DOWNWARD, max(limit, vsf->limit));
        }
        case UPWARD: {
            return value_set::top;
        }
      }
    }
    case UPWARD: {
      switch(vsf->open_dir) {
        case DOWNWARD: {
          return value_set::top;
        }
        case UPWARD: {
          return make_shared<vs_open>(DOWNWARD, min(limit, vsf->limit));
        }
      }
    }
  }
}

//          if(limit < vsf->limit && vsf->limit - limit <= 100) {
//            vs_finite::elements_t elements;
//            for(int64_t i = limit; i <= vsf->limit; i++)
//              elements.insert(i);
//            return make_shared<vs_finite>(elements);
//          } else

void summy::vs_open::accept(value_set_visitor &v) {
  v.visit(this);
}
