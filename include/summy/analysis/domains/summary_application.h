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
#include <summy/value_set/value_set.h>
#include <tuple>

namespace analysis {

class summary_application {
private:
  summary_memory_state *caller;
  summary_memory_state *summary;

  std::experimental::optional<summary_memory_state*> return_site;
  std::map<id_shared_t, ptr_set_t, id_less> ptr_map;
  typedef std::set<id_shared_t, id_less> alias_queue_t;
  alias_queue_t ptr_worklist;

  ptr_set_t offsets_bytes_to_bits_base(int64_t base, ptr_set_t const &region_keys_c);
  void build_pmap_region(id_shared_t region_key_summary, ptr_set_t const &region_keys_c, summary_memory_state::regions_getter_t rgetter);
public:
  summary_application(summary_memory_state *caller, summary_memory_state *summary);

  summary_memory_state *apply_summary();
};

}
