/*
 * summary_application.h
 *
 *  Created on: Jun 28, 2016
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/domains/summary_memory_state.h>
#include <summy/analysis/domains/numeric/numeric_state.h>
#include <summy/analysis/domains/ptr_set.h>
#include <summy/analysis/domains/api/numeric/num_var.h>
#include <tuple>

namespace analysis {

class summary_application {
  summary_memory_state *caller;
  summary_memory_state *summary;

  std::experimental::optional<summary_memory_state*> return_site;
  map<id_shared_t, ptr_set_t, id_less> ptr_map;
public:
  summary_application(summary_memory_state *caller, summary_memory_state *summary);

  summary_memory_state *apply_summary();
};

}
