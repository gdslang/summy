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

#pragma once

namespace analysis {

typedef std::function<std::experimental::optional<bool>(size_t const&, size_t const&)> node_compare_t;
typedef std::unordered_set<size_t> fpp_elements_t;

extern node_compare_t node_compare_default;

class fp_priority_queue {
private:
  std::vector<node_compare_t> comparers;
  fpp_elements_t inner;
public:
  fp_priority_queue();
  fp_priority_queue(std::set<size_t> init);
  fp_priority_queue(std::vector<node_compare_t> node_comparers);
  fp_priority_queue(std::set<size_t> init, std::vector<node_compare_t> node_comparers);

  void push(size_t value);
  size_t pop();
  bool empty();
  void clear();
  size_t size();
};

}
