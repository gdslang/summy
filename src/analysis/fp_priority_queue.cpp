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
#include <experimental/optional>

using std::experimental::optional;

using namespace std;
using namespace analysis;

node_compare_t analysis::node_compare_default = [](size_t const &a, size_t const &b) { return a < b; };

analysis::fp_priority_queue::fp_priority_queue() : comparers({node_compare_default}) {}

analysis::fp_priority_queue::fp_priority_queue(std::set<size_t> init) : comparers({node_compare_default}) {
  inner.insert(init.begin(), init.end());
}

analysis::fp_priority_queue::fp_priority_queue(std::vector<node_compare_t> node_comparers) : comparers(node_comparers) {}

analysis::fp_priority_queue::fp_priority_queue(std::set<size_t> init, std::vector<node_compare_t> node_comparers)
    : comparers(node_comparers) {
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
    optional<bool> result;
    for(auto &comparer : comparers) {
      if(result)
        break;
      result = comparer(*it, minimum);
    }
    assert(result);
    if(result.value()) {
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
