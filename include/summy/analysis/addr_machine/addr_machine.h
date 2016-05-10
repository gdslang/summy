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
#include <summy/analysis/addr_machine/addr_machine_state.h>
#include <memory>

namespace analysis {
namespace addr_machine {

typedef std::vector<std::shared_ptr<addr_machine_state>> state_t;

struct addr_machine_result : public ::analysis::analysis_result<state_t> {
  addr_machine_result(state_t &s) : analysis_result(s) {
  }
};

class addr_machine: public fp_analysis {
private:
  state_t state;

  virtual void add_constraint(size_t from, size_t to, const ::cfg::edge *e);
  virtual void remove_constraint(size_t from, size_t to);
  virtual dependency gen_dependency(size_t from, size_t to);
  virtual void init_state();
public:
  addr_machine(cfg::cfg *cfg);
  ~addr_machine();

  std::shared_ptr<addr_machine_state> bottom();
  std::shared_ptr<addr_machine_state> start_value(size_t node);

  std::shared_ptr<domain_state> get(size_t node);
  void update(size_t node, std::shared_ptr<domain_state> state);
  addr_machine_result result();

  void put(std::ostream &out);
};

}  // namespace addr
}  // namespace analysis
