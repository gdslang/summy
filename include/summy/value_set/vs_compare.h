/*
 * vs_compare.h
 *
 *  Created on: Feb 20, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include "value_set.h"

namespace summy {

struct vs_total_less {
  bool operator()(vs_shared_t a, vs_shared_t b) const;
};

}
