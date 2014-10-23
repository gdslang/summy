/*
 * liveness.h
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/analysis.h>
#include <summy/analysis/liveness/lv_elem.h>
#include <summy/cfg/edge.h>
#include <memory>
#include <vector>
#include <set>

namespace analysis {
namespace liveness {

typedef std::vector<std::shared_ptr<lv_elem>> state_t;
typedef std::function<std::shared_ptr<lv_elem>()> constraint_t;
typedef std::vector<std::vector<singleton_t>> newly_live_vector_t;

struct liveness_result : public ::analysis::analysis_result<state_t> {
  newly_live_vector_t &pn_newly_live;

  liveness_result(state_t &s, newly_live_vector_t &pn_newly_live) :
      analysis_result(s), pn_newly_live(pn_newly_live) {
  }

  bool contains(size_t node_id, singleton_key_t sk, unsigned long long offset, unsigned long long size);
  bool contains(size_t node_id, singleton_t s);
};

class liveness : public analysis {
private:
  state_t state;
  newly_live_vector_t pn_newly_live;

  virtual void add_constraint(size_t from, size_t to, const ::cfg::edge *e);
  virtual void remove_constraint(size_t from, size_t to);
  virtual void add_dependency(size_t from, size_t to);
  virtual void remove_dependency(size_t from, size_t to);
public:
  liveness(cfg::cfg *cfg);
  ~liveness();

  shared_ptr<lattice_elem> bottom();

  shared_ptr<lattice_elem> get(size_t node);
  void update(size_t node, shared_ptr<lattice_elem> state);
  liveness_result result();

  void put(std::ostream &out);
};

}  // namespace reaching_defs
}  // namespace analysis
