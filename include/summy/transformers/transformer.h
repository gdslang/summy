/*
 * transformer.h
 *
 *  Created on: Sep 16, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/cfg/cfg.h>

class transformer {
protected:
  cfg::cfg *cfg;
public:
  transformer(cfg::cfg *cfg) :
      cfg(cfg) {
  }
  virtual ~transformer() {
  }

  virtual void transform() = 0;
};
