/*
 * fp_priority_queue.cpp
 *
 *  Created on: Apr 23, 2015
 *      Author: Julian Kranz
 */
#include <summy/analysis/fp_priority_queue.h>

using namespace std;
using namespace analysis;

static node_compare_t node_compare_default = [](size_t a, size_t b) {
  return a < b;
};

analysis::fp_priority_queue::fp_priority_queue() : inner(node_compare_default) {
}

analysis::fp_priority_queue::fp_priority_queue(std::set<size_t> init) : inner(node_compare_default) {
  inner.insert(init.begin(), init.end());
}

analysis::fp_priority_queue::fp_priority_queue(node_compare_t node_compare) : inner(node_compare) {
}

analysis::fp_priority_queue::fp_priority_queue(std::set<size_t> init, node_compare_t node_compare) : inner(node_compare) {
  inner.insert(init.begin(), init.end());
}

void analysis::fp_priority_queue::push(size_t value) {
  inner.insert(value);
}

size_t analysis::fp_priority_queue::pop() {
  size_t value;
  auto it = inner.begin();
  value = *it;
  inner.erase(it);
  return value;
}

bool analysis::fp_priority_queue::empty() {
  return inner.empty();
}

void analysis::fp_priority_queue::clear() {
  inner.clear();
}
