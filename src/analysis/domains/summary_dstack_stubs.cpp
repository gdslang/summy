/*
 * sms_stubs.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: Julian Kranz
 */

#include <summy/analysis/domain_state.h>
#include <summy/analysis/domains/api/numeric/num_var.h>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/analysis/domains/summary_dstack_stubs.h>
#include <summy/rreil/id/memory_id.h>
#include <summy/rreil/id/special_ptr.h>
#include <summy/value_set/vs_finite.h>

using analysis::api::num_var;
using summy::rreil::allocation_memory_id;
using summy::rreil::special_ptr;
using summy::vs_finite;

using namespace std;
using namespace analysis;

shared_ptr<summary_memory_state> analysis::summary_dstack_stubs::allocator(size_t allocation_site, size_t size) {
  shared_ptr<summary_memory_state> malloc_summary = shared_ptr<summary_memory_state>(summary_dstack::sms_top(sm));

  id_shared_t rax = id_shared_t(new gdsl::rreil::arch_id("A"));
  io_region rax_region = malloc_summary->region_by_id(&relation::get_regions, rax);

//  auto allocation_site_ct = [=](id_shared_t nid_in) {
//    return ptr(shared_ptr<gdsl::rreil::id>(new allocation_memory_id(allocation_site)), vs_finite::zero);
//  };

  field &f_out = rax_region.insert(malloc_summary->child_state, 0, 64, false);
  num_var fout_var(f_out.num_id);
  ptr _nullptr = ptr(special_ptr::_nullptr, vs_finite::zero);
  ptr alloc_ptr = ptr(shared_ptr<gdsl::rreil::id>(new allocation_memory_id(allocation_site)), vs_finite::zero);
  malloc_summary->child_state->assign(&fout_var, {_nullptr, alloc_ptr});

  return malloc_summary;
}
