/*
 * fixpoint.cpp
 *
 *  Created on: Sep 24, 2014
 *      Author: Julian Kranz
 */


#include <summy/analysis/fixpoint.h>
#include <summy/analysis/analysis.h>
#include <queue>

using namespace std;

void analysis::fixpoint::iterate() {
  queue<size_t> worklist = analysis->initial();
  while(!worklist.empty()) {
    size_t node_id = worklist.front();
    worklist.pop();


  }
}
