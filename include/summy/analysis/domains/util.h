/*
 * util.h
 *
 *  Created on: Jul 9, 2016
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/ptr_set.h>

namespace analysis {
ptr unpack_singleton(ptr_set_t aliases);
}

