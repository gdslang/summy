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

#pragma once

namespace analysis {

typedef std::function<bool(size_t, size_t)> node_compare_t;
typedef std::unordered_set<size_t> fpp_elements_t;

class fp_priority_queue {
private:
  node_compare_t comparer;
  fpp_elements_t inner;
public:
  fp_priority_queue();
  fp_priority_queue(std::set<size_t> init);
  fp_priority_queue(node_compare_t node_compare);
  fp_priority_queue(std::set<size_t> init, node_compare_t node_compare);

  void push(size_t value);
  size_t pop();
  bool empty();
  void clear();
  size_t size();
};

}
