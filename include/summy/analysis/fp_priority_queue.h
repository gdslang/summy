/*
 * fp_priority_queue.h
 *
 *  Created on: Apr 23, 2015
 *      Author: Julian Kranz
 */
#include <set>
#include <unordered_set>
#include <functional>
#include <stdlib.h>
#include <experimental/optional>
#include <vector>
#include <summy/analysis/fp_analysis.h>

#pragma once

namespace analysis {

//typedef std::unordered_set<analysis_node> fpp_elements_t;
typedef std::set<analysis_node> fpp_elements_t;

extern node_compare_t node_compare_default;

class fp_priority_queue {
private:
  std::vector<node_compare_t> comparers;
  fpp_elements_t inner;
public:
  fp_priority_queue();
  fp_priority_queue(std::set<analysis_node> init);
  fp_priority_queue(std::vector<node_compare_t> node_comparers);
  fp_priority_queue(std::set<analysis_node> init, std::vector<node_compare_t> node_comparers);

  void push(size_t value);
  size_t pop();
  bool empty();
  void clear();
  size_t size();
};

}
