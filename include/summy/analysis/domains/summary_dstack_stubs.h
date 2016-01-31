/*
 * sms_stubs.h
 *
 *  Created on: Jan 7, 2016
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/summary_memory_state.h>
#include <memory>

namespace analysis {

class summary_dstack_stubs {
private:
  shared_ptr<static_memory> sm;
  bool warnings;

public:
  summary_dstack_stubs(shared_ptr<static_memory> sm, bool warnings) : sm(sm), warnings(warnings) {}

  std::shared_ptr<summary_memory_state> allocator(size_t allocation_site, size_t size);
  std::shared_ptr<summary_memory_state> no_effect();
};
}
