/*
 * observer.cpp
 *
 *  Created on: Oct 23, 2014
 *      Author: Julian Kranz
 */

#include <summy/cfg/observer.h>
#include <summy/cfg/cfg.h>

cfg::observer::observer(cfg::cfg *cfg) {
  this->cfg = cfg;
  cfg->register_observer(this);
}

cfg::observer::~observer() {
  cfg->unregister_observer(this);
}
