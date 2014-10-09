/*
 * util.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/util.h>

bool analysis::id_less::operator ()(std::shared_ptr<gdsl::rreil::id> a,
    std::shared_ptr<gdsl::rreil::id> b) {
  return a->to_string().compare(b->to_string()) < 0;
}
