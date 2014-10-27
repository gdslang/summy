/*
 * renamer.h
 *
 *  Created on: Oct 17, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/transformers/transformer.h>
#include <summy/analysis/adaptive_rd/adaptive_rd.h>
#include <summy/cfg/edge.h>
#include <summy/cfg/observer.h>
#include <vector>

class renamer: public transformer, public cfg::observer {
public:
private:
  struct update_task {
    cfg::edge *e;
    size_t from;
    size_t to;
  };

  analysis::adaptive_rd::adaptive_rd_result rd_result;
  void task_from_edge(std::vector<update_task> &tasks, size_t from, size_t to, const cfg::edge *e);
  void transform(std::vector<update_task> &tasks);
public:
  renamer(cfg::cfg *cfg, analysis::adaptive_rd::adaptive_rd_result rd_result) :
      transformer(cfg), rd_result(rd_result) {
  }

  virtual void transform();
  void notify(std::vector<cfg::update> const &updates);
};
