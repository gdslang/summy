/*
 * util.cpp
 *
 *  Created on: Jul 9, 2016
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/util.h>
#include <summy/rreil/id/special_ptr.h>
#include <optional>
#include <assert.h>
#include <summy/analysis/domains/numeric/als_state.h>

using std::optional;
using summy::rreil::special_ptr;

using namespace analysis;

ptr analysis::unpack_singleton(ptr_set_t aliases) {
  aliases = als_state::normalise(aliases);
  optional<ptr> opt_result;
  optional<ptr> bad_result;
  for(auto &alias : aliases) {
    if(*alias.id == *special_ptr::_nullptr) continue;
    if(*alias.id == *special_ptr::badptr) {
      bad_result = alias;
      continue;
    }
    assert(!opt_result);
    opt_result = alias;
  }
  if(opt_result)
    return opt_result.value();
  else
    return bad_result.value();
}
