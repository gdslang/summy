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
private:
  class cfg *cfg;
public:
  observer(class cfg *cfg);
  virtual ~observer();

  virtual void notify(std::vector<update> const &updates) = 0;
};
}
