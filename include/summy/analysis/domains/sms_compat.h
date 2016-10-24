/*
 * sms_compat.h
 *
 *  Created on: Jul 9, 2016
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/api/numeric/num_var.h>
#include <summy/analysis/domains/numeric/numeric_state.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <tuple>

namespace analysis {


class sms_compat {
private:
  static std::tuple<bool, num_var_pairs_t, api::id_set_t> compatMatchSeparate(bool widening, relation &a_in, relation &a_out, numeric_state *a_n, relation &b_in,
    relation &b_out, numeric_state *b_n);

public:
  /**
   * This function tries to establish a mapping between pointers by structurally matching
   * the input of the given summaries. If a pointer is only found in one of the summaries,
   * the respective pointer is added to the other summary. This way, all pointers are matched
   * as long as there are no conflicts, that is, partially overlapping fields.
   *
   * @return pair of variables that correspond to each other in the respective memory states
   */
  static std::tuple<bool, memory_head, numeric_state *, numeric_state *> compat(
    bool widening, summary_memory_state const *a, summary_memory_state const *b);
};

}
