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
#include <summy/cfg/observer.h>

namespace cfg {

enum jump_dir {
  BACKWARD, FORWARD, UNKNOWN
};

class jd_manager : public observer {
private:
  /*
   * Map each node to its machine address
   */
  analysis::addr::addr addr;
  analysis::fixpoint fp;

  bool notified;
public:
  jd_manager(::cfg::cfg *cfg);

  jump_dir jump_direction(size_t from, size_t to);
  void notify(std::vector<update> const &updates);
};

}
