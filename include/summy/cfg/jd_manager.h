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
#include <summy/cfg/observer.h>
#include "cfg.h"

namespace cfg {

enum jump_dir {
  BACKWARD, FORWARD, UNKNOWN
};

class jd_manager : observer {
private:
  ::cfg::cfg *cfg;

  /*
   * Map each node to its machine address
   */
  std::map<size_t, size_t> address_map;
public:
  jd_manager(::cfg::cfg *cfg);

  jump_dir jump_direction(size_t from, size_t to);

  void notify(std::vector<update> const &updates);
};

}
