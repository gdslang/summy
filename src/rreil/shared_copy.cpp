/*
 * shared_copy.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: Julian Kranz
 */
#include <summy/rreil/shared_copy.h>
#include <summy/rreil/copy_visitor.h>

using summy::rreil::copy_visitor;
using namespace std;

std::shared_ptr<gdsl::rreil::id> shared_copy(gdsl::rreil::id *id) {
  copy_visitor cv;
  id->accept(cv);
  return shared_ptr<gdsl::rreil::id>(cv.get_id());
}
