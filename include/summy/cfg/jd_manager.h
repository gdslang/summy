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
#include <summy/analysis/addr_machine/addr_machine.h>
#include <summy/analysis/fixpoint.h>
#include <summy/cfg/edge/edge.h>
#include "cfg.h"
#include <summy/cfg/observer.h>

namespace cfg {

class jd_manager : public observer {
private:
  /*
   * Map each node to its machine address
   */
  analysis::addr_machine::addr_machine addr;
  analysis::fixpoint fp;

  bool notified;
  ::cfg::cfg *cfg;
public:
  jd_manager(::cfg::cfg *cfg);

  jump_dir jump_direction(size_t from, size_t to);
  size_t machine_address_of(size_t node);
  std::shared_ptr<analysis::addr_machine::addr_machine_state> address_of(size_t node);
  void notify(std::vector<update> const &updates);
};

}
