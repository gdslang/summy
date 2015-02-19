/*
 * num_linear.h
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/domains/numeric/api/num_var.h>
#include <summy/value_set/value_set.h>
#include <iosfwd>

namespace analysis {
namespace numeric {

class num_linear {
private:
  virtual void put(std::ostream &out) = 0;

public:
  virtual ~num_linear() {
  }

  friend std::ostream &operator<< (std::ostream &out, num_linear &_this);
};

std::ostream& operator<<(std::ostream &out, num_linear &_this);

class num_linear_term : public num_linear {
private:
  int64_t scale;
  num_var *var;
  num_linear *next;

  virtual void put(std::ostream &out);
public:
  num_linear_term(int64_t scale, num_var *var, num_linear *next) :
      scale(scale), var(var), next(next) {
  }
  num_linear_term(num_var *var, num_linear *next) : num_linear_term(1, var, next) {
  }
  num_linear_term(int64_t scale, num_var *var);
  num_linear_term(num_var *var) : num_linear_term(scale, var) {
  }
};

class num_linear_vs : public num_linear {
private:
  summy::vs_shared_t value_set;

  virtual void put(std::ostream &out);
public:
  num_linear_vs(summy::vs_shared_t value_set) : value_set(value_set) {
  }
};

}
}
