/*
 * util.h
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <cppgdsl/rreil/id/id.h>
#include <memory>
#include <string>
#include <summy/rreil/id/id_visitor.h>
#include <experimental/optional>

#include <vector>

namespace analysis {

std::string print_id_no_version(std::shared_ptr<gdsl::rreil::id> x);

/**
 * Test two id instances for equality. The version of IDs (see ssa_id)
 * is ignored.
 */
struct id_less_no_version {
  bool operator()(std::shared_ptr<gdsl::rreil::id> a, std::shared_ptr<gdsl::rreil::id> b) const;
};

struct id_less {
  bool operator()(std::shared_ptr<gdsl::rreil::id> a, std::shared_ptr<gdsl::rreil::id> b) const;
};

}
