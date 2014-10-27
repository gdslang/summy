/*
 * observer.h
 *
 *  Created on: Oct 23, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <vector>

namespace cfg {

struct update;
class cfg;

class observer {
public:
  observer();
  virtual ~observer();

  virtual void notify(std::vector<update> const &updates) = 0;
};
}
