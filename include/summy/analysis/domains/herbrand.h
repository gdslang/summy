#pragma once

#include <summy/analysis/domains/mempath.h>
#include <set>

namespace analysis {

using fptr_query_t = mempath;
using alias_conflict_query_t = std::set<mempath>;

}
