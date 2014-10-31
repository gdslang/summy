/*
 * goto_ip_adder.h
 *
 *  Created on: Sep 23, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include "transformer.h"

class goto_ip_adder : public transformer {
public:
  using transformer::transformer;

  virtual void transform();
};
