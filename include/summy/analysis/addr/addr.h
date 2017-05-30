/*
 * addr.h
 *
 *  Created on: Dec 22, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/analysis/fp_analysis.h>
#include <summy/cfg/cfg.h>
#include <vector>
#include <functional>
#include <set>
#include <map>
#include <ostream>
#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/addr/addr_state.h>
#include <memory>

namespace analysis {
namespace addr {

typedef std::vector<std::shared_ptr<addr_state>> state_t;

struct addr_result : public ::analysis::analysis_result<state_t> {
  addr_result(state_t &s) : analysis_result(s) {
  }
};

class addr: public fp_analysis {
private:
  state_t state;
  std::map<size_t, size_t> addr_virt_counter_map;
  get_next_virt_t get_next_virt;

  std::map<size_t, std::shared_ptr<domain_state>> transform(
    size_t from, size_t to, const ::cfg::edge *e, size_t from_ctx);
  virtual dependency gen_dependency(size_t from, size_t to);
  virtual void init_state();
public:
  addr(cfg::cfg *cfg);
  ~addr();

  std::shared_ptr<addr_state> bottom();
  std::shared_ptr<domain_state> start_state(size_t node);

  std::shared_ptr<domain_state> get(size_t node);
  void update(analysis_node node, std::shared_ptr<domain_state> state);
  addr_result result();

  void put(std::ostream &out);
};

}  // namespace addr
}  // namespace analysis
