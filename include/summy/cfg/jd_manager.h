/*
 * jd_manager.h
 *
 *  Created on: Apr 10, 2015
 *      Author: Julian Kranz
 */

/*
 * Manage the diretion (forward or backward) of jumps
 */

#pragma once
#include <summy/analysis/addr/addr.h>
#include <summy/analysis/fixpoint.h>
#include "cfg.h"

namespace cfg {

enum jump_dir {
  BACKWARD, FORWARD, UNKNOWN
};

class jd_manager {
private:
  /*
   * Map each node to its machine address
   */
  analysis::addr::addr addr;
  analysis::fixpoint fp;
public:
  jd_manager(::cfg::cfg *cfg);

  jump_dir jump_direction(size_t from, size_t to);
};

}
