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
  using transformer::transformer;

  virtual void transform();
};
