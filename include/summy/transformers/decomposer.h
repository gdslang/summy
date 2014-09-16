/*
 * decomposer.h
 *
 *  Created on: Aug 20, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include "transformer.h"

class decomposer: public transformer {
public:
  decomposer(cfg::cfg *cfg) :
      transformer(cfg) {
  }

  virtual void transform();
};
