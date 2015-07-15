/*
 * sms_op.h
 *
 *  Created on: Jul 15, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/domains/summary_memory_state.h>

namespace analysis {

struct summary_application_t {
  summary_memory_state *return_site;
  /*
   * Todo: Information about unexpected aliases?
   */
};
summary_memory_state *apply_summary(summary_memory_state *caller, summary_memory_state *summary);
}
