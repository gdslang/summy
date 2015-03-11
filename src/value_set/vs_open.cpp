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

vs_shared_t summy::vs_open::narrow(const vs_finite *vsf) const {
  return make_shared<vs_finite>(*vsf);
}

vs_shared_t summy::vs_open::narrow(const vs_open *vsf) const {
  return make_shared<vs_open>(*this);
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

vs_shared_t summy::vs_open::add(const vs_finite *vs) const {
  return vs->add(this);
}

vs_shared_t summy::vs_open::add(const vs_open *vs) const {
  auto mk = [&]() {
    return make_shared<vs_open>(open_dir, limit + vs->limit);
  };
  switch(open_dir) {
    case DOWNWARD: {
      switch(vs->open_dir) {
        case DOWNWARD: {
          return mk();
        }
        case UPWARD: {
          return top;
        }
      }
      break;
    }
    case UPWARD: {
      switch(vs->open_dir) {
        case DOWNWARD: {
          return top;
        }
        case UPWARD: {
          return mk();
        }
      }
    }
  }
}

vs_shared_t summy::vs_open::neg() const {
  switch(open_dir) {
    case DOWNWARD: {
      return make_shared<vs_open>(UPWARD, -limit);
    }
    case UPWARD: {
      return make_shared<vs_open>(DOWNWARD, -limit);
    }
  }
}

vs_shared_t summy::vs_open::mul(const vs_finite *vs) const {
  return vs->mul(this);
}

vs_shared_t summy::vs_open::mul(const vs_open *vs) const {
  if(!one_sided() || !vs->one_sided())
    return top;
  auto mk = [&](auto open_dir) {
    return make_shared<vs_open>(open_dir, limit*vs->limit);
  };
  if(open_dir == vs->open_dir)
    return mk(UPWARD);
  else
    return mk(DOWNWARD);
}

vs_shared_t summy::vs_open::div(const vs_finite *vs) const {
  if(sgn(vs->min()) != sgn(vs->max()))
    return top;
  int64_t sign = sgn(vs->min());
  switch(open_dir) {
    case DOWNWARD: {
      if(sign < 0) {
        int64_t min_abs = -vs->min();
        return make_shared<vs_open>(UPWARD, limit / min_abs);
      } else {
        int64_t max = -vs->max();
        return make_shared<vs_open>(DOWNWARD, limit / max);
      }
    }
    case UPWARD: {
      if(sign < 0) {
        int64_t min_abs = -vs->min();
        return make_shared<vs_open>(DOWNWARD, limit / min_abs);
      } else {
        int64_t max = -vs->max();
        return make_shared<vs_open>(UPWARD, limit / max);
      }
    }
  }
}

vs_shared_t summy::vs_open::div(const vs_open *vs) const {
  if(!vs->one_sided())
    return top;
  if(vs->get_limit() == 0) {
    if(vs->get_open_dir() == DOWNWARD)
      return neg();
    else
      return make_shared<vs_open>(*this);
  }
  switch(open_dir) {
    case DOWNWARD: {
      switch(vs->open_dir) {
        case DOWNWARD: {
          return make_shared<vs_open>(UPWARD, min((int64_t)0, -limit));
        }
        case UPWARD: {
          return make_shared<vs_open>(DOWNWARD, max((int64_t)0, limit));
        }
      }
      break;
    }
    case UPWARD: {
      switch(vs->open_dir) {
        case DOWNWARD: {
          return make_shared<vs_open>(DOWNWARD, max((int64_t)0, -limit));
        }
        case UPWARD: {
          return make_shared<vs_open>(UPWARD, min((int64_t)0, limit));
        }
      }
    }
  }
}

vs_shared_t summy::vs_open::div(const vs_top *vs) const {
  return top;
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
      break;
    }
    case UPWARD: {
      switch(vsf->open_dir) {
        case DOWNWARD: {
          return value_set::top;
        }
        case UPWARD: {
          return make_shared<vs_open>(UPWARD, min(limit, vsf->limit));
        }
      }
    }
  }
}

void summy::vs_open::accept(value_set_visitor &v) {
  v.visit(this);
}

bool summy::vs_open::one_sided() const {
  switch(open_dir) {
    case DOWNWARD:
      return sgn(limit) <= 0;
    case UPWARD:
      return sgn(limit) >= 0;
  }
}
