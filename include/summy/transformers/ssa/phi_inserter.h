/*
 * phi_inserter.h
 *
 *  Created on: Oct 17, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/transformers/transformer.h>
#include <summy/analysis/adaptive_rd/adaptive_rd.h>
#include <summy/cfg/observer.h>
#include <summy/cfg/phi_edge.h>
#include <vector>

class phi_inserter: public transformer, public cfg::observer {
private:
  struct phi_task {
    cfg::phi_edge *pe;
    size_t from;
    size_t to;
  };

  analysis::adaptive_rd::adaptive_rd_result rd_result;

  void task_from_edge(std::vector<phi_task> &tasks, size_t from, size_t to);
  void transform(std::vector<phi_task> &tasks);
public:
  phi_inserter(cfg::cfg *cfg, analysis::adaptive_rd::adaptive_rd_result rd_result) :
      transformer(cfg), rd_result(rd_result) {
  }

  virtual void transform();
  void notify(std::vector<cfg::update> const &updates);
};
