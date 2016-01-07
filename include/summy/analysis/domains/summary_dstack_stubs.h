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
public:
  std::shared_ptr<summary_memory_state> allocator(void *allocation_site, size_t size);
};

}
