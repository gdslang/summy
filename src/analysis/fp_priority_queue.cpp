/*
 * fp_priority_queue.cpp
 *
 *  Created on: Apr 23, 2015
 *      Author: Julian Kranz
 */
#include <summy/analysis/fp_priority_queue.h>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace analysis;

static node_compare_t node_compare_default = [](size_t a, size_t b) {
  return a < b;
};

analysis::fp_priority_queue::fp_priority_queue() : comparer(node_compare_default) {
}

analysis::fp_priority_queue::fp_priority_queue(std::set<size_t> init) : comparer(node_compare_default) {
  inner.insert(init.begin(), init.end());
}

analysis::fp_priority_queue::fp_priority_queue(node_compare_t node_compare) : comparer(node_compare) {
}

analysis::fp_priority_queue::fp_priority_queue(std::set<size_t> init, node_compare_t node_compare) :
    comparer(node_compare) {
  inner.insert(init.begin(), init.end());
}

void analysis::fp_priority_queue::push(size_t value) {
  inner.insert(value);
}

size_t analysis::fp_priority_queue::pop() {
//  for(auto e : inner)
//    cout << "===> " << e << endl;
  auto min_it = std::min_element(inner.begin(),inner.end(), comparer);
  size_t minimum = *min_it;
  inner.erase(min_it);
  return minimum;
}

bool analysis::fp_priority_queue::empty() {
  return inner.empty();
}

void analysis::fp_priority_queue::clear() {
  inner.clear();
}
