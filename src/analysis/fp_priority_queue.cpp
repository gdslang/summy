/*
 * fp_priority_queue.cpp
 *
 *  Created on: Apr 23, 2015
 *      Author: Julian Kranz
 */
#include <summy/analysis/fp_priority_queue.h>
#include <algorithm>
#include <iostream>
#include <assert.h>

using namespace std;
using namespace analysis;

static node_compare_t node_compare_default = [](size_t const &a, size_t const &b) { return a < b; };

analysis::fp_priority_queue::fp_priority_queue() : comparer(node_compare_default) {}

analysis::fp_priority_queue::fp_priority_queue(std::set<size_t> init) : comparer(node_compare_default) {
  inner.insert(init.begin(), init.end());
}

analysis::fp_priority_queue::fp_priority_queue(node_compare_t node_compare) : comparer(node_compare) {}

analysis::fp_priority_queue::fp_priority_queue(std::set<size_t> init, node_compare_t node_compare)
    : comparer(node_compare) {
  inner.insert(init.begin(), init.end());
}

void analysis::fp_priority_queue::push(size_t value) {
  inner.insert(value);
}

size_t analysis::fp_priority_queue::pop() {
  //  for(auto e : inner)
  //    cout << "===> " << e << endl;
  assert(inner.size() > 0);
  size_t minimum = SIZE_MAX;
  auto min_it = inner.begin();
  auto it = inner.begin();
  while(it != inner.end()) {
    if(*it < minimum) {
      minimum = *it;
      min_it = it;
    }
    it++;
  }
  //  auto min_it = std::min_element(inner.begin(), inner.end(), comparer);
  //  size_t minimum = *min_it;
  inner.erase(min_it);
  return minimum;
}

bool analysis::fp_priority_queue::empty() {
  return inner.empty();
}

void analysis::fp_priority_queue::clear() {
  inner.clear();
}

size_t analysis::fp_priority_queue::size() {
  return inner.size();
}
