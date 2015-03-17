/*
 * liveness.h
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/liveness/lv_state.h>
#include <summy/analysis/fp_analysis.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <memory>
#include <vector>
#include <set>
#include <functional>

namespace analysis {
namespace liveness {

typedef std::vector<std::shared_ptr<lv_state>> state_t;
typedef std::function<std::shared_ptr<lv_state>()> constraint_t;
typedef std::map<size_t, std::vector<singleton_t>> newly_live_t;
typedef cfg::edge_payload_map_t<bool> edge_bool_map_t;

/*
 * Todo: const
 */
struct liveness_result : public ::analysis::analysis_result<state_t> {
  newly_live_t &pn_newly_live;
  edge_bool_map_t &edge_liveness;

  liveness_result(state_t &s, newly_live_t &pn_newly_live, edge_bool_map_t &edge_liveness) :
      analysis_result(s), pn_newly_live(pn_newly_live), edge_liveness(edge_liveness) {
  }

  bool contains(size_t node_id, singleton_key_t sk, unsigned long long offset, unsigned long long size);
  bool contains(size_t node_id, singleton_t s);
};

class liveness : public fp_analysis {
private:
  state_t state;
  newly_live_t pn_newly_live;
  edge_bool_map_t edge_liveness;
//  std::map<int, bool> edge_liveness;

  virtual void add_constraint(size_t from, size_t to, const ::cfg::edge *e);
  virtual void remove_constraint(size_t from, size_t to);
  virtual dependency gen_dependency(size_t from, size_t to);
  virtual void init_state();
public:
  liveness(cfg::cfg *cfg);
  ~liveness();

  shared_ptr<domain_state> bottom();

  shared_ptr<domain_state> get(size_t node);
  void update(size_t node, shared_ptr<domain_state> state);
  liveness_result result();

  void put(std::ostream &out);
};

}  // namespace liveness
}  // namespace analysis
