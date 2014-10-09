/*
 * lv_elem.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/liveness/lv_elem.h>

using namespace analysis::liveness;

analysis::liveness::lv_elem::~lv_elem() {
}

lv_elem* analysis::liveness::lv_elem::lub(::analysis::lattice_elem* other) {
}

lv_elem* analysis::liveness::lv_elem::add(living_t ids) {
}

lv_elem* analysis::liveness::lv_elem::remove(living_t ids) {
}

bool analysis::liveness::lv_elem::operator >(::analysis::lattice_elem& other) {
}
