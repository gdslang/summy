/*
 * util.h
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <cppgdsl/rreil/id/id.h>
#include <memory>

namespace analysis {

struct id_less {
  bool operator()(std::shared_ptr<gdsl::rreil::id> a, std::shared_ptr<gdsl::rreil::id> b);
};

}
