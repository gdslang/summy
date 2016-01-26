/*
 * sms_op.h
 *
 *  Created on: Jul 15, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <tuple>
#include <summy/analysis/domains/summary_memory_state.h>
#include <summy/analysis/domains/numeric/numeric_state.h>
#include <summy/analysis/domains/ptr_set.h>

namespace analysis {

ptr unpack_singleton(ptr_set_t aliases);

struct summary_application_t {
  summary_memory_state *return_site;
  /*
   * Todo: Information about unexpected aliases?
   */
};
summary_memory_state *apply_summary(summary_memory_state *caller, summary_memory_state *summary);

/**
 * This function tries to establish a mapping between pointers by structurally matching
 * the input of the given summaries. If a pointer is only found in one of the summaries,
 * the respective pointer is added to the other summary. This way, all pointers are matched
 * as long as there are no conflicts, that is, partially overlapping fields.
 *
 * @return pair of variables that correspond to each other in the respective memory states
 */
num_var_pairs_t compatMatchSeparate(
  relation &a_in, relation &a_out, numeric_state *a_n, relation &b_in, relation &b_out, numeric_state *b_n);
std::tuple<memory_head, numeric_state *, numeric_state *> compat(
  summary_memory_state const *a, summary_memory_state const *b);
}
