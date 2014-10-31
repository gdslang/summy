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
  cfg::cfg_view cfg_view;
public:
  transformer(cfg::cfg *cfg) :
      cfg(cfg), cfg_view(cfg) {
  }
  transformer(cfg::cfg *cfg, size_t root) :
      cfg(cfg), cfg_view(cfg, root) {
  }
  virtual ~transformer() {
  }

  void set_root(size_t root) {
    cfg_view = cfg::cfg_view(cfg, root);
  }

  virtual void transform() = 0;
};
