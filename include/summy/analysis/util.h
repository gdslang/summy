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

std::string print_id_no_version(gdsl::rreil::id const &x);

/**
 * Test two id instances for equality. The version of IDs (see ssa_id)
 * is ignored.
 */
struct id_less_no_version {
  bool operator()(std::shared_ptr<gdsl::rreil::id> const &a, std::shared_ptr<gdsl::rreil::id> const &b) const;
};

struct id_less {
  bool operator()(std::shared_ptr<gdsl::rreil::id> const &a, std::shared_ptr<gdsl::rreil::id> const &b) const;

  bool operator()(gdsl::rreil::id const &a, std::shared_ptr<gdsl::rreil::id> const &b) const;

  bool operator()(std::shared_ptr<gdsl::rreil::id> const &a, gdsl::rreil::id const &b) const;

  bool operator()(gdsl::rreil::id const &a, gdsl::rreil::id const &b) const;
};

} // namespace analysis
