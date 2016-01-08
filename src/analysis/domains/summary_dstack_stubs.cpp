/*
 * sms_stubs.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: Julian Kranz
 */

#include <summy/analysis/domain_state.h>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/analysis/domains/summary_dstack_stubs.h>
#include <summy/rreil/id/memory_id.h>
#include <summy/value_set/vs_finite.h>

using summy::rreil::allocation_memory_id;
using summy::vs_finite;

using namespace std;
using namespace analysis;

shared_ptr<summary_memory_state> analysis::summary_dstack_stubs::allocator(void *allocation_site, size_t size) {
  shared_ptr<summary_memory_state> malloc_summary = shared_ptr<summary_memory_state>(summary_dstack::sms_top(sm));

  id_shared_t rax = id_shared_t(new gdsl::rreil::arch_id("A"));
  io_region rax_region = malloc_summary->region_by_id(&relation::get_regions, rax);

  auto allocation_site_ct = [=](id_shared_t nid_in) {
    return ptr(shared_ptr<gdsl::rreil::id>(new allocation_memory_id(allocation_site)), vs_finite::zero);
  };

  rax_region.insert(malloc_summary->child_state, 0, 64, false, allocation_site_ct);

  return malloc_summary;
}
