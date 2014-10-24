/*
 * big_step.h
 *
 *  Created on: Oct 24, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/observer.h>

class big_step : public cfg::observer {
private:
  cfg::cfg *cfg;
public:
  big_step(cfg::cfg *cfg);
  virtual ~big_step() {
  }

  virtual void transduce() = 0;
};
